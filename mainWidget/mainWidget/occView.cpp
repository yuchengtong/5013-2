#include <OpenGl_GraphicDriver.hxx>
#include "occView.h"

#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>
#include <QShowEvent>
#include <QPainter>
#include <QDebug>

#include <V3d_View.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <WNT_Window.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>

static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
    static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
    return aGraphicDriver;
}

OccView::OccView(QWidget* parent)
    : QWidget(parent),
    myXmin(0),
    myYmin(0),
    myXmax(0),
    myYmax(0),
    myCurrentMode(CurAction3d_DynamicRotation),
    myDegenerateModeIsOn(Standard_True),
    myRectBand(NULL),
    myIsInitialized(false)
{
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);

    // 关键属性组合：这三个必须同时设置
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);




    setMouseTracking(true);
}

OccView::~OccView()
{
    if (!myView.IsNull()) {
        myView->Remove();
    }
    if (!myContext.IsNull()) {
        myContext->RemoveAll(Standard_True);
    }
}

void OccView::showEvent(QShowEvent* theEvent)
{
    QWidget::showEvent(theEvent);

    if (!myIsInitialized && width() > 0 && height() > 0) {
        qDebug() << "OccView::showEvent - initializing, size:" << width() << "x" << height();
        init();
        myIsInitialized = true;
    }

    if (myIsInitialized && !myView.IsNull()) {
        myView->MustBeResized();
        myView->Redraw();
    }
}

QPaintEngine* OccView::paintEngine() const
{
    return 0;
}

void OccView::paintEvent(QPaintEvent* /*theEvent*/)
{
    if (!myView.IsNull()) {
        myView->Redraw();
    }
    else {
        // 关键：OCC未初始化时显示红色背景，证明widget存在
        QPainter painter(this);
        painter.fillRect(rect(), Qt::red);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "OCC Not Ready");
    }
}

void OccView::resizeEvent(QResizeEvent* /*theEvent*/)
{
    if (!myView.IsNull()) {
        myView->MustBeResized();
        myView->Redraw();
    }
}

void OccView::init()
{
    qDebug() << "OccView::init() started";

    try {
        Handle(Aspect_DisplayConnection) aDisplayConnection = new Aspect_DisplayConnection();

        if (GetGraphicDriver().IsNull()) {
            qDebug() << "Creating new OpenGl_GraphicDriver...";
            GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
        }

        // 强制确保原生窗口已创建
        WId window_handle = (WId)winId();
        qDebug() << "Window handle:" << window_handle;

        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)window_handle);
        qDebug() << "WNT_Window created, mapped:" << wind->IsMapped();

        myViewer = new V3d_Viewer(GetGraphicDriver());
        myView = myViewer->CreateView();
        myView->SetWindow(wind);

        if (!wind->IsMapped()) {
            wind->Map();
            qDebug() << "WNT_Window mapped";
        }

        myContext = new AIS_InteractiveContext(myViewer);

        myViewer->SetDefaultLights();

        Handle(V3d_DirectionalLight) aDir = new V3d_DirectionalLight(
            V3d_XnegYnegZpos,
            Quantity_Color(1.0, 1.0, 1.0, Quantity_TOC_RGB),
            Standard_False
        );
        aDir->SetIntensity(0.8f);
        myViewer->AddLight(aDir);
        myViewer->SetLightOn(aDir);  // 单独开启，确保生效

        // 补一个环境光，避免背光面死黑
        Handle(V3d_AmbientLight) aAmb = new V3d_AmbientLight(
            Quantity_Color(0.6, 0.6, 0.6, Quantity_TOC_RGB)
        );
        aAmb->SetIntensity(0.4f);
        myViewer->AddLight(aAmb);
        myViewer->SetLightOn(aAmb);


        myViewer->SetLightOn();

        Quantity_Color a(45.0 / 255, 86.0 / 255, 126.0 / 255, Quantity_TOC_RGB);
        Quantity_Color b(159.0 / 255, 160.0 / 255, 162.0 / 255, Quantity_TOC_RGB);
        myView->SetBgGradientColors(a, b, Aspect_GFM_VER);

        myView->MustBeResized();
        myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

        myContext->SetDisplayMode(AIS_Shaded, Standard_True);

        qDebug() << "OccView::init() completed successfully";

        // 强制立即重绘
        myView->Redraw();

        SetCameraRotationState(false);
        myView->SetProj(V3d_Zneg);
        myView->SetTwist(-M_PI / 2.0);


        update();

    }
    catch (Standard_Failure& e) {
        qDebug() << "OccView::init() EXCEPTION:" << e.GetMessageString();
    }
    catch (...) {
        qDebug() << "OccView::init() UNKNOWN EXCEPTION";
    }
}

// ====== 以下保持你原有代码不变 ======

const Handle(AIS_InteractiveContext)& OccView::getContext() const
{
    return myContext;
}

void OccView::SetCameraRotationState(bool isRotation)
{
    b_IsRatotion = isRotation;
}

bool OccView::GetCameraRotationState()
{
    return b_IsRatotion;
}

void OccView::fitAll(void)
{
    if (!myView.IsNull()) {
        myView->FitAll();
        myView->ZFitAll();
        myView->Redraw();
    }
}

void OccView::reset(void)
{
    if (!myView.IsNull()) {
        myView->Reset();
        myView->FitAll();
        myView->ZFitAll();
        myView->Redraw();
    }
}

void OccView::pan(void)
{
    myCurrentMode = CurAction3d_DynamicPanning;
}

void OccView::zoom(void)
{
    myCurrentMode = CurAction3d_DynamicZooming;
}

void OccView::rotate(void)
{
    myCurrentMode = CurAction3d_DynamicRotation;
}

void OccView::mousePressEvent(QMouseEvent* theEvent)
{
    if (theEvent->button() == Qt::LeftButton)
        onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    else if (theEvent->button() == Qt::MidButton)
        onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    else if (theEvent->button() == Qt::RightButton)
        onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
}

void OccView::mouseReleaseEvent(QMouseEvent* theEvent)
{
    if (theEvent->button() == Qt::LeftButton)
        onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    else if (theEvent->button() == Qt::MidButton)
        onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    else if (theEvent->button() == Qt::RightButton)
        onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
}

void OccView::mouseMoveEvent(QMouseEvent* theEvent)
{
    onMouseMove(theEvent->buttons(), theEvent->pos());
}

void OccView::wheelEvent(QWheelEvent* theEvent)
{
    onMouseWheel(theEvent->buttons(), theEvent->delta(), theEvent->pos());
}

void OccView::onLButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();
}

void OccView::onMButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();

    if (myCurrentMode == CurAction3d_DynamicRotation && !myView.IsNull())
    {
        myView->StartRotation(thePoint.x(), thePoint.y());
    }
}

void OccView::onRButtonDown(const int /*theFlags*/, const QPoint /*thePoint*/)
{
}

void OccView::onMouseWheel(const int /*theFlags*/, const int theDelta, const QPoint thePoint)
{
    if (myView.IsNull()) return;

    Standard_Integer aFactor = 16;
    Standard_Integer aX = thePoint.x();
    Standard_Integer aY = thePoint.y();

    if (theDelta > 0) {
        aX += aFactor;
        aY += aFactor;
    }
    else {
        aX -= aFactor;
        aY -= aFactor;
    }
    myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
}

void OccView::addItemInPopup(QMenu* /*theMenu*/)
{
}

void OccView::popup(const int /*x*/, const int /*y*/)
{
}

void OccView::onLButtonUp(const int theFlags, const QPoint thePoint)
{
    if (myRectBand) myRectBand->hide();

    if (thePoint.x() == myXmin && thePoint.y() == myYmin) {
        if (theFlags & Qt::ControlModifier)
            multiInputEvent(thePoint.x(), thePoint.y());
        else
            inputEvent(thePoint.x(), thePoint.y());
    }
}

void OccView::onMButtonUp(const int /*theFlags*/, const QPoint thePoint)
{
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
        panByMiddleButton(thePoint);
}

void OccView::onRButtonUp(const int /*theFlags*/, const QPoint thePoint)
{
    popup(thePoint.x(), thePoint.y());
}

void OccView::onMouseMove(const int theFlags, const QPoint thePoint)
{
    if (theFlags & Qt::LeftButton) {
        drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());
        dragEvent(thePoint.x(), thePoint.y());
    }

    if (theFlags & Qt::ControlModifier)
        multiMoveEvent(thePoint.x(), thePoint.y());
    else
        moveEvent(thePoint.x(), thePoint.y());

    if (theFlags & Qt::MidButton) {
        switch (myCurrentMode) {
        case CurAction3d_DynamicRotation:
            if (b_IsRatotion && !myView.IsNull())
                myView->Rotation(thePoint.x(), thePoint.y());
            break;
        case CurAction3d_DynamicZooming:
            if (!myView.IsNull())
                myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
            break;
        case CurAction3d_DynamicPanning:
            if (!myView.IsNull()) {
                myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
                myXmax = thePoint.x();
                myYmax = thePoint.y();
            }
            break;
        default:
            break;
        }
    }
}

void OccView::dragEvent(const int x, const int y)
{
    if (myContext.IsNull()) return;
    myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);
    emit selectionChanged();
}

void OccView::multiDragEvent(const int x, const int y)
{
    if (myContext.IsNull()) return;
    myContext->ShiftSelect(myXmin, myYmin, x, y, myView, Standard_True);
    emit selectionChanged();
}

void OccView::inputEvent(const int x, const int y)
{
    Q_UNUSED(x); Q_UNUSED(y);
    if (myContext.IsNull()) return;
    myContext->Select(Standard_True);
    emit selectionChanged();
}

void OccView::multiInputEvent(const int x, const int y)
{
    Q_UNUSED(x); Q_UNUSED(y);
    if (myContext.IsNull()) return;
    myContext->ShiftSelect(Standard_True);
    emit selectionChanged();
}

void OccView::moveEvent(const int x, const int y)
{
    if (!myContext.IsNull() && !myView.IsNull())
        myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::multiMoveEvent(const int x, const int y)
{
    if (!myContext.IsNull() && !myView.IsNull())
        myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::drawRubberBand(const int minX, const int minY, const int maxX, const int maxY)
{
    QRect aRect;
    (minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
    (minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));
    aRect.setWidth(abs(maxX - minX));
    aRect.setHeight(abs(maxY - minY));

    if (!myRectBand) {
        myRectBand = new QRubberBand(QRubberBand::Rectangle, this);
        myRectBand->setStyle(QStyleFactory::create("windows"));
    }
    myRectBand->setGeometry(aRect);
    myRectBand->show();
}

void OccView::panByMiddleButton(const QPoint& thePoint)
{
    if (myView.IsNull()) return;

    Standard_Integer aCenterX = size().width() / 2;
    Standard_Integer aCenterY = size().height() / 2;
    myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}