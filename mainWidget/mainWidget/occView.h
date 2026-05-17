#ifndef _OCCVIEW_H_
#define _OCCVIEW_H_

#include <QWidget>
#include <AIS_InteractiveContext.hxx>
#include <TopoDS_Shape.hxx>

class QMenu;
class QRubberBand;
class QShowEvent;

class OccView : public QWidget
{
    Q_OBJECT

public:
    enum CurrentAction3d
    {
        CurAction3d_Nothing,
        CurAction3d_DynamicZooming,
        CurAction3d_WindowZooming,
        CurAction3d_DynamicPanning,
        CurAction3d_GlobalPanning,
        CurAction3d_DynamicRotation
    };

    OccView(QWidget* parent);
    ~OccView();

    const Handle(AIS_InteractiveContext)& getContext() const;
    const Handle(V3d_View)& getView() const { return myView; }
    TopoDS_Shape& GetShape() { return m_Shape; }

    void SetCameraRotationState(bool isRotation);
    bool GetCameraRotationState();

signals:
    void selectionChanged(void);

public slots:
    void pan(void);
    void fitAll(void);
    void reset(void);
    void zoom(void);
    void rotate(void);

protected:
    virtual QPaintEngine* paintEngine() const;

    virtual void paintEvent(QPaintEvent* theEvent);
    virtual void resizeEvent(QResizeEvent* theEvent);
    virtual void showEvent(QShowEvent* theEvent);

    virtual void mousePressEvent(QMouseEvent* theEvent);
    virtual void mouseReleaseEvent(QMouseEvent* theEvent);
    virtual void mouseMoveEvent(QMouseEvent* theEvent);
    virtual void wheelEvent(QWheelEvent* theEvent);

    virtual void onLButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onRButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMouseWheel(const int theFlags, const int theDelta, const QPoint thePoint);
    virtual void onLButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onRButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMouseMove(const int theFlags, const QPoint thePoint);

    virtual void addItemInPopup(QMenu* theMenu);

protected:
    void init(void);
    void popup(const int x, const int y);
    void dragEvent(const int x, const int y);
    void inputEvent(const int x, const int y);
    void moveEvent(const int x, const int y);
    void multiMoveEvent(const int x, const int y);
    void multiDragEvent(const int x, const int y);
    void multiInputEvent(const int x, const int y);
    void drawRubberBand(const int minX, const int minY, const int maxX, const int maxY);
    void panByMiddleButton(const QPoint& thePoint);

private:
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;

    Standard_Integer myXmin;
    Standard_Integer myYmin;
    Standard_Integer myXmax;
    Standard_Integer myYmax;

    CurrentAction3d myCurrentMode;
    Standard_Boolean myDegenerateModeIsOn;
    QRubberBand* myRectBand;
    TopoDS_Shape m_Shape;
    bool b_IsRatotion = true;

    bool myIsInitialized = false;
};

#endif // _OCCVIEW_H_