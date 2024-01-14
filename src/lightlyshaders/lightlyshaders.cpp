/*
 *   Copyright © 2015 Robert Metsäranta <therealestrob@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  if not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "lightlyshaders.h"
#include "lightlyshaders_config.h"
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QWindow>
#include <opengl/glutils.h>
#include <effect/effect.h>
#include <core/renderviewport.h>
#include <QMatrix4x4>
#include <KConfigGroup>
#include <QRegularExpression>
#include <QBitmap>
#include <KWindowEffects>

Q_LOGGING_CATEGORY(LIGHTLYSHADERS, "kwin_effect_lightlyshaders", QtWarningMsg)

static void ensureResources()
{
    // Must initialize resources manually because the effect is a static lib.
    Q_INIT_RESOURCE(lightlyshaders);
}

namespace KWin {

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(  LightlyShadersEffect, 
                                        "lightlyshaders.json", 
                                        return LightlyShadersEffect::supported();, 
                                        return LightlyShadersEffect::enabledByDefault();)

LightlyShadersEffect::LightlyShadersEffect() : OffscreenEffect()
{
    ensureResources();

    m_helper = new LSHelper();
    reconfigure(ReconfigureAll);

    m_shader = std::unique_ptr<GLShader>(ShaderManager::instance()->generateShaderFromFile(ShaderTrait::MapTexture, QStringLiteral(""), QStringLiteral(":/effects/lightlyshaders/shaders/lightlyshaders.frag")));

    if (!m_shader) {
        qCWarning(LIGHTLYSHADERS) << "Failed to load shader";
        return;
    }

    if (m_shader->isValid())
    {
        const int sampler = m_shader->uniformLocation("sampler");
        const int expanded_size = m_shader->uniformLocation("expanded_size");
        const int frame_size = m_shader->uniformLocation("frame_size");
        const int csd_shadow_offset = m_shader->uniformLocation("csd_shadow_offset");
        const int radius = m_shader->uniformLocation("radius");
        const int shadow_sample_offset = m_shader->uniformLocation("shadow_sample_offset");
        const int outline_strength = m_shader->uniformLocation("outline_strength");
        const int draw_outline = m_shader->uniformLocation("draw_outline");
        const int dark_theme = m_shader->uniformLocation("dark_theme");
        const int squircle_ratio = m_shader->uniformLocation("squircle_ratio");
        const int is_squircle = m_shader->uniformLocation("is_squircle");
        ShaderManager::instance()->pushShader(m_shader.get());
        m_shader->setUniform(is_squircle, 10);
        m_shader->setUniform(squircle_ratio, 9);
        m_shader->setUniform(dark_theme, 8);
        m_shader->setUniform(draw_outline, 7);
        m_shader->setUniform(outline_strength, 6);
        m_shader->setUniform(shadow_sample_offset, 5);
        m_shader->setUniform(radius, 4);
        m_shader->setUniform(csd_shadow_offset, 3);
        m_shader->setUniform(frame_size, 2);
        m_shader->setUniform(expanded_size, 1);
        m_shader->setUniform(sampler, 0);
        ShaderManager::instance()->popShader();

        const auto stackingOrder = effects->stackingOrder();
        for (EffectWindow *window : stackingOrder) {
            windowAdded(window);
        }

        connect(effects, &EffectsHandler::windowAdded, this, &LightlyShadersEffect::windowAdded);
        connect(effects, &EffectsHandler::windowDeleted, this, &LightlyShadersEffect::windowDeleted);

        qCWarning(LIGHTLYSHADERS) << "LightlyShaders loaded.";
    }
    else
        qCWarning(LIGHTLYSHADERS) << "LightlyShaders: no valid shaders found! LightlyShaders will not work.";
}

LightlyShadersEffect::~LightlyShadersEffect()
{
    m_windows.clear();
}

void
LightlyShadersEffect::windowDeleted(EffectWindow *w)
{
    m_windows.remove(w);
}

void
LightlyShadersEffect::windowAdded(EffectWindow *w)
{
    m_windows[w].isManaged = false;

    if(!m_helper->isManagedWindow(w))
        return;

    m_windows[w].isManaged = true;
    m_windows[w].skipEffect = false;

    connect(w, &EffectWindow::windowMaximizedStateChanged, this, &LightlyShadersEffect::windowMaximizedStateChanged);

    QRectF maximized_area = effects->clientArea(MaximizeArea, w);
    if (maximized_area == w->frameGeometry() && m_disabledForMaximized)
        m_windows[w].skipEffect = true;
    
    redirect(w);
    setShader(w, m_shader.get());
}

void 
LightlyShadersEffect::windowMaximizedStateChanged(EffectWindow *w, bool horizontal, bool vertical) 
{
    if (!m_disabledForMaximized) return;

    if ((horizontal == true) && (vertical == true)) {
        m_windows[w].skipEffect = true;
    } else {
        m_windows[w].skipEffect = false;
    }
}

void
LightlyShadersEffect::setRoundness(const int r, Output *s)
{
    m_size = r;
    m_screens[s].sizeScaled = r*m_screens[s].scale;
    m_corner = QSize(m_size+(m_shadowOffset-1), m_size+(m_shadowOffset-1));
}

void
LightlyShadersEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    LightlyShadersConfig::self()->load();

    m_alpha = LightlyShadersConfig::alpha();
    m_outline = LightlyShadersConfig::outline();
    m_darkTheme = LightlyShadersConfig::darkTheme();
    m_disabledForMaximized = LightlyShadersConfig::disabledForMaximized();
    m_shadowOffset = LightlyShadersConfig::shadowOffset();
    m_squircleRatio = LightlyShadersConfig::squircleRatio();
    m_cornersType = LightlyShadersConfig::cornersType();

    m_helper->reconfigure();
    m_roundness = m_helper->roundness();

    if(m_shadowOffset>=m_roundness) {
        m_shadowOffset = m_roundness-1;
    }

    const auto screens = effects->screens();
    for(Output *s : screens)
    {
        if (effects->waylandDisplay() == nullptr) {
            s = nullptr;
        }
        setRoundness(m_roundness, s);

        if (effects->waylandDisplay() == nullptr) {
            break;
        }
    }

    effects->addRepaintFull();
}

void
LightlyShadersEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const QRegion &region, Output *s)
{
    bool set_roundness = false;

    qreal scale = viewport.scale();

    if(scale != m_screens[s].scale) {
        m_screens[s].scale = scale;
        set_roundness = true;
    }

    if(set_roundness) {
        setRoundness(m_roundness, s);
        m_helper->reconfigure();
    } 

    effects->paintScreen(renderTarget, viewport, mask, region, s);
}

void
LightlyShadersEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds time)
{
    if (!isValidWindow(w) )
    {
        effects->prePaintWindow(w, data, time);
        return;
    }

    Output *s = w->screen();
    if (effects->waylandDisplay() == nullptr) {
        s = nullptr;
    } 

    const QRectF geo(w->frameGeometry());
    for (int corner = 0; corner < LSHelper::NTex; ++corner)
    {
        QRegion reg = QRegion(scale(m_helper->m_maskRegions[corner]->boundingRect(), m_screens[s].scale).toRect());
        switch(corner) {
            case LSHelper::TopLeft:
                reg.translate(geo.x()-m_shadowOffset, geo.y()-m_shadowOffset);
                break;
            case LSHelper::TopRight:
                reg.translate(geo.x() + geo.width() - m_size, geo.y()-m_shadowOffset);
                break;
            case LSHelper::BottomRight:
                reg.translate(geo.x() + geo.width() - m_size-1, geo.y()+geo.height()-m_size-1); 
                break;
            case LSHelper::BottomLeft:
                reg.translate(geo.x()-m_shadowOffset+1, geo.y()+geo.height()-m_size-1);
                break;
            default:
                break;
        }
        
        data.opaque -= reg;
    }

    effects->prePaintWindow(w, data, time);
}

bool
LightlyShadersEffect::isValidWindow(EffectWindow *w)
{
    if (!m_shader->isValid()
            //|| (!w->isOnCurrentDesktop() && !(mask & PAINT_WINDOW_TRANSFORMED))
            //|| w->isMinimized()
            || !m_windows[w].isManaged
            //|| effects->hasActiveFullScreenEffect()
            || m_windows[w].skipEffect
        )
    {
        return false;
    }
    return true;
}

void
LightlyShadersEffect::drawWindow(const RenderTarget &renderTarget, const RenderViewport &viewport, EffectWindow *w, int mask, const QRegion &region, WindowPaintData &data)
{    
    QRectF screen = viewport.renderRect().toRect();

    if (!isValidWindow(w) || (!screen.intersects(w->frameGeometry()) && !(mask & PAINT_WINDOW_TRANSFORMED)) )
    {
        effects->drawWindow(renderTarget, viewport, w, mask, region, data);
        return;
    }

    Output *s = w->screen();
    if (effects->waylandDisplay() == nullptr) {
        s = nullptr;
    }

    QRectF geo(w->frameGeometry());
    QRectF exp_geo(w->expandedGeometry());
    QRectF contents_geo(w->contentsRect());
    //geo.translate(data.xTranslation(), data.yTranslation());
    const QRectF geo_scaled = scale(geo, m_screens[s].scale);
    const QRectF exp_geo_scaled = scale(exp_geo, m_screens[s].scale);

    //Draw rounded corners with shadows   
    //const int mvpMatrixLocation = m_shader->uniformLocation("modelViewProjectionMatrix");
    const int frameSizeLocation = m_shader->uniformLocation("frame_size");
    const int expandedSizeLocation = m_shader->uniformLocation("expanded_size");
    const int csdShadowOffsetLocation = m_shader->uniformLocation("csd_shadow_offset");
    const int radiusLocation = m_shader->uniformLocation("radius");
    const int shadowOffsetLocation = m_shader->uniformLocation("shadow_sample_offset");
    const int outlineStrengthLocation = m_shader->uniformLocation("outline_strength");
    const int drawOutlineLocation = m_shader->uniformLocation("draw_outline");
    const int darkThemeLocation = m_shader->uniformLocation("dark_theme");
    const int squircleRatioLocation = m_shader->uniformLocation("squircle_ratio");
    const int isSquircleLocation = m_shader->uniformLocation("is_squircle");
    ShaderManager *sm = ShaderManager::instance();
    sm->pushShader(m_shader.get());

    //qCWarning(LIGHTLYSHADERS) << geo_scaled.width() << geo_scaled.height();
    m_shader->setUniform(frameSizeLocation, QVector2D(geo_scaled.width(), geo_scaled.height()));
    m_shader->setUniform(expandedSizeLocation, QVector2D(exp_geo_scaled.width(), exp_geo_scaled.height()));
    m_shader->setUniform(csdShadowOffsetLocation, QVector3D(geo_scaled.x() - exp_geo_scaled.x(), geo_scaled.y()-exp_geo_scaled.y(), exp_geo_scaled.height() - geo_scaled.height() - geo_scaled.y() + exp_geo_scaled.y() ));
    m_shader->setUniform(radiusLocation, m_screens[s].sizeScaled);
    m_shader->setUniform(shadowOffsetLocation, m_shadowOffset);
    m_shader->setUniform(outlineStrengthLocation, float(m_alpha)/100);
    m_shader->setUniform(drawOutlineLocation, m_outline);
    m_shader->setUniform(darkThemeLocation, m_darkTheme);
    m_shader->setUniform(squircleRatioLocation, m_squircleRatio);
    m_shader->setUniform(isSquircleLocation, (m_cornersType == LSHelper::SquircledCorners));

    glActiveTexture(GL_TEXTURE0);
    
    OffscreenEffect::drawWindow(renderTarget, viewport, w, mask, region, data);

    sm->popShader();
}

QRectF
LightlyShadersEffect::scale(const QRectF rect, qreal scaleFactor)
{
    return QRectF(
        rect.x()*scaleFactor,
        rect.y()*scaleFactor,
        rect.width()*scaleFactor,
        rect.height()*scaleFactor
    );
}

bool
LightlyShadersEffect::enabledByDefault()
{
    return supported();
}

bool
LightlyShadersEffect::supported()
{
    return effects->isOpenGLCompositing() && GLFramebuffer::supported();
}

} // namespace KWin

#include "lightlyshaders.moc"