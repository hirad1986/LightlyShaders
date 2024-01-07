#include "lshelper.h"
#include "lightlyshaders_config.h"

#include <QBitmap>
#include <QPainter>
#include <QPainterPath>

namespace KWin {

LSHelper::LSHelper() : QObject()
{
    for (int i = 0; i < NTex; ++i)
    {
        m_maskRegions[i] = 0;
    }
}

LSHelper::~LSHelper()
{
    //delete mask regions
    for (int i = 0; i < NTex; ++i)
    {
        if (m_maskRegions[i])
            delete m_maskRegions[i];
    }
}

void
LSHelper::reconfigure()
{
    LightlyShadersConfig::self()->load();

    m_cornersType = LightlyShadersConfig::cornersType();
    m_squircleRatio = LightlyShadersConfig::squircleRatio();
    m_shadowOffset = LightlyShadersConfig::shadowOffset();
    m_size = LightlyShadersConfig::roundness();
    m_disabledForMaximized = LightlyShadersConfig::disabledForMaximized();

    setMaskRegions();
}

void
LSHelper::setMaskRegions()
{
    int size = m_size + m_shadowOffset;
    QImage img = genMaskImg(size, true, false);

    m_maskRegions[TopLeft] = createMaskRegion(img, size, TopLeft);
    m_maskRegions[TopRight] = createMaskRegion(img, size, TopRight);
    m_maskRegions[BottomRight] = createMaskRegion(img, size, BottomRight);
    m_maskRegions[BottomLeft] = createMaskRegion(img, size, BottomLeft);
}

QRegion *
LSHelper::createMaskRegion(QImage img, int size, int corner)
{
    QImage img_copy;

    switch(corner) {
        case TopLeft:
            img_copy = img.copy(0, 0, size, size);
            break;
        case TopRight:
            img_copy = img.copy(size, 0, size, size);
            break;
        case BottomRight:
            img_copy = img.copy(size, size, size, size);
            break;
        case BottomLeft:
            img_copy = img.copy(0, size, size, size);
            break;
    }

    img_copy = img_copy.createMaskFromColor(QColor(Qt::black).rgb(), Qt::MaskOutColor);
    QBitmap bitmap = QBitmap::fromImage(img_copy, Qt::DiffuseAlphaDither);

    return new QRegion(bitmap);
}

void 
LSHelper::roundBlurRegion(EffectWindow *w, QRegion *blur_region)
{
    if(blur_region->isEmpty()) {
        return;
    }

    const QRectF geo(w->frameGeometry());

    QRectF maximized_area = effects->clientArea(MaximizeArea, w);
    if (maximized_area == geo && m_disabledForMaximized) {
        return;
    }

	QRegion top_left = *m_maskRegions[TopLeft];
    top_left.translate(0-m_shadowOffset+1, 0-m_shadowOffset+1);  
    *blur_region = blur_region->subtracted(top_left);  
    
    QRegion top_right = *m_maskRegions[TopRight];
    top_right.translate(geo.width() - m_size-1, 0-m_shadowOffset+1);   
    *blur_region = blur_region->subtracted(top_right);  

    QRegion bottom_right = *m_maskRegions[BottomRight];
    bottom_right.translate(geo.width() - m_size-1, geo.height()-m_size-1);    
    *blur_region = blur_region->subtracted(bottom_right);     
    
    QRegion bottom_left = *m_maskRegions[BottomLeft];
    bottom_left.translate(0-m_shadowOffset+1, geo.height()-m_size-1);
    *blur_region = blur_region->subtracted(bottom_left);
}

QPainterPath
LSHelper::drawSquircle(float size, int translate)
{
    QPainterPath squircle;
    float squircleSize = size * 2 * (float(m_squircleRatio)/24.0 * 0.25 + 0.8); //0.8 .. 1.05
    float squircleEdge = (size * 2) - squircleSize;

    squircle.moveTo(size, 0);
    squircle.cubicTo(QPointF(squircleSize, 0), QPointF(size * 2, squircleEdge), QPointF(size * 2, size));
    squircle.cubicTo(QPointF(size * 2, squircleSize), QPointF(squircleSize, size * 2), QPointF(size, size * 2));
    squircle.cubicTo(QPointF(squircleEdge, size * 2), QPointF(0, squircleSize), QPointF(0, size));
    squircle.cubicTo(QPointF(0, squircleEdge), QPointF(squircleEdge, 0), QPointF(size, 0));

    squircle.translate(translate,translate);

    return squircle;
}

QImage
LSHelper::genMaskImg(int size, bool mask, bool outer_rect)
{
    QImage img(size*2, size*2, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    QRect r(img.rect());
    int offset_decremented;
    if(outer_rect) {
        offset_decremented = m_shadowOffset-1;
    } else {
        offset_decremented = m_shadowOffset;
    }

    if(mask) {
        p.fillRect(img.rect(), Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setRenderHint(QPainter::Antialiasing);
        if (m_cornersType == SquircledCorners) {
            const QPainterPath squircle1 = drawSquircle((size-m_shadowOffset), m_shadowOffset);
            p.drawPolygon(squircle1.toFillPolygon());
        } else {
            p.drawEllipse(r.adjusted(m_shadowOffset,m_shadowOffset,-m_shadowOffset,-m_shadowOffset));
        }
    } else {
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        r.adjust(offset_decremented, offset_decremented, -offset_decremented, -offset_decremented);
        if(outer_rect) {
            p.setBrush(QColor(0, 0, 0, 255));
        } else {
            p.setBrush(QColor(255, 255, 255, 255));
        }
        if (m_cornersType == SquircledCorners) {
            const QPainterPath squircle2 = drawSquircle((size-offset_decremented), offset_decremented);
            p.drawPolygon(squircle2.toFillPolygon());
        } else {
            p.drawEllipse(r);
        }
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        r.adjust(1, 1, -1, -1);
        if (m_cornersType == SquircledCorners) {
            const QPainterPath squircle3 = drawSquircle((size-(offset_decremented+1)), (offset_decremented+1));
            p.drawPolygon(squircle3.toFillPolygon());
        } else {
            p.drawEllipse(r);
        }
    }
    p.end();

    return img;
}

}