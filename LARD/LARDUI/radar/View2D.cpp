#include "View2d.h"
#include "Scene2D.h"
#include <QApplication>
#include <sstream>
#include <QDebug>
#include <QtMath>
#include "XYScene2D.h"




template<> View2D* Singleton<View2D>::m_singleton = 0;
View2D::View2D(QWidget* parent,Scene2D* scene,float zoom_step,float margin) : 
QWidget(parent),
m_render(new QPainter),
m_scene(scene),
m_last_cursor(0.0f,0.0f),
m_zoom_step(zoom_step),
m_margin(margin),
m_zoom_scale(1.0f),
m_scene_offset_x(0.0f),
m_scene_offset_y(0.0f),
m_rect_scaling(false),
m_translating_label(false),
m_clear_map(":/Resources/images/cleanport.png"),
m_hide_map(":/Resources/images/open.png"),
m_left_mouse_location (0.0F,0.0F)

{
	m_scene->SetView(this);
	m_scene_size_x = m_scene->GetSize().width();
	m_scene_size_y = m_scene->GetSize().height(); 
	m_view_matrix.reset();
	setMinimumSize(QSize(600,600));
	UpdateViewMatrix();
	setAutoFillBackground(false);
	this->setFocusPolicy( Qt::ClickFocus );
	m_right_mouse_down = false;
	m_right_mouse_move = false;
	m_right_mouse_up	  = false;
	m_main_menu = new QMenu(this);


	bDis_plot_radar1 = FALSE;
	bDis_plot_radar2 = FALSE;
	bDis_plot_radar3 = FALSE;
	bDis_track = FALSE;
	b_dispGPSinfo = FALSE;
	b_leftbutton_press = FALSE;
	b_right_mouse_scal = false;
	m_left_mouse_angle = 0.0;
	m_left_mouse_dist   = 0.0;

    m_width = 20;
    m_height = 50;




    initShipPoint();
}

View2D::~View2D()
{
	delete m_render;
	m_render = NULL;

}

void View2D::resizeEvent(QResizeEvent* event )
{
	UpdateViewMatrix();
	Draw();
        initShipPoint();
	QWidget::resizeEvent(event);
}

QPainter* View2D::GetRender()
{
	return m_render;
}

void View2D::paintEvent(QPaintEvent* event)
{
	m_render->begin(this);
	m_render->setRenderHint(QPainter::Antialiasing,true);
    m_render->fillRect(QRectF(QPointF(0.0,0.0),this->size()),BACKGROUND_BRUSH);
	m_scene->Draw();
    DrawScaleRect();


    //




    //绘制提示信息
    QPainter painter(this);
    QFont font1("宋体",12,QFont::Bold);
    painter.setPen(QColor(40,129,75));
    painter.setFont(font1);
    //左上
    painter.drawText(QRect(10,10,300,20),"NAME:VICTORIA",Qt::AlignVCenter|Qt::AlignLeft);
    painter.drawText(QRect(10,35,300,20),"IMO:UN1234567",Qt::AlignVCenter|Qt::AlignLeft);

    //右上
    painter.drawText(QRect(this->width()-300-10,10,300,20),"HEADING:137",Qt::AlignVCenter|Qt::AlignRight);
    painter.drawText(QRect(this->width()-300-10,35,300,20),"UTC:16:55:21",Qt::AlignVCenter|Qt::AlignRight);
    painter.drawText(QRect(this->width()-300-10,60,300,20),"22Nov 2018",Qt::AlignVCenter|Qt::AlignRight);
    //左下
    painter.drawText(QRect(10,this->height()-10-70,300,20),"PORT LRAD",Qt::AlignVCenter|Qt::AlignLeft);
    painter.drawText(QRect(10,this->height()-10-45,300,20),"PAN:315 AUTO",Qt::AlignVCenter|Qt::AlignLeft);
    painter.drawText(QRect(10,this->height()-10-20,300,20),"TITLE:-2",Qt::AlignVCenter|Qt::AlignLeft);

    //右下
    painter.drawText(QRect(this->width()-300-10,this->height()-10-70,300,20),"STBD LRAD",Qt::AlignVCenter|Qt::AlignRight);
    painter.drawText(QRect(this->width()-300-10,this->height()-10-45,300,20),"PAN:072 MANUAL",Qt::AlignVCenter|Qt::AlignRight);
    painter.drawText(QRect(this->width()-300-10,this->height()-10-20,300,20),"TITLE:-2",Qt::AlignVCenter|Qt::AlignRight);



    DrawShip(&painter);

	m_render->end();


}

Scene2D* View2D::GetScene()
{
	return m_scene;
}

const QMatrix& View2D::GetViewMatrix() const
{
	return m_view_matrix;
}

void View2D::mousePressEvent (QMouseEvent* event)
{
    qDebug()<<event->pos();
    m_last_cursor = event->pos();
	if (event->button() == Qt::LeftButton)
	{
		QMatrix inverted = m_view_matrix.inverted();
        QPointF curr_in_scene = inverted.map(event->pos());
		m_left_mouse_angle = atan2(curr_in_scene.x(),curr_in_scene.y())*180/MY_PI;
		if (m_left_mouse_angle<0)
		{
			m_left_mouse_angle = m_left_mouse_angle +360;
		}
		m_left_mouse_dist = sqrt(curr_in_scene.x() * curr_in_scene.x()+curr_in_scene.y()*curr_in_scene.y());
		if (m_scene->HitTest(m_last_cursor))//取消左键移动视图
		{
			Draw();
		}
		m_left_mouse_location =  QCursor::pos();
        b_leftbutton_press = TRUE;

	}
	else if(event->button() == Qt::RightButton)
	{
		m_scale_rect_left_top     = m_last_cursor;
		m_scale_rect_right_bottom = m_last_cursor;
		m_right_mouse_down         = true;
	}
}

void View2D::mouseMoveEvent(QMouseEvent* event)
{
	if (QApplication::mouseButtons().testFlag(Qt::LeftButton))
	{
		this->setCursor( Qt::ClosedHandCursor );
//		QMatrix inverted = m_view_matrix.inverted();
//        QPointF curr_in_scene = inverted.map(event->pos());
//		QPointF last_in_scene = inverted.map(m_last_cursor);
//		m_scene_offset_x += last_in_scene.x() - curr_in_scene.x();
//		m_scene_offset_y += last_in_scene.y() - curr_in_scene.y();
//        m_last_cursor = event->pos();
//		UpdateViewMatrix();
		Draw();
		b_leftbutton_press = FALSE;

	}
	else if (QApplication::mouseButtons().testFlag(Qt::RightButton))
	{
		m_rect_scaling = true;
        m_scale_rect_right_bottom = event->pos();
		Draw();
		m_right_mouse_move = true;
	}
}

void View2D::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (m_translating_label)
		{
			m_translating_label = false;
		}
		b_leftbutton_press = FALSE;
		this->setCursor(Qt::ArrowCursor);
	}
	else if (event->button() == Qt::RightButton)
	{
		if (m_rect_scaling)
		{
			m_rect_scaling = false;
			QPointF center_in_scene = m_view_matrix.inverted().map( (m_scale_rect_left_top +  m_scale_rect_right_bottom ) / 2.0f );
			m_scene_offset_x = center_in_scene.x();
			m_scene_offset_y = center_in_scene.y();
			float rect_w = abs(m_scale_rect_right_bottom.x() - m_scale_rect_left_top.x());
			float rect_h = abs(m_scale_rect_right_bottom.y() - m_scale_rect_left_top.y());
			if (rect_w > 0.0f && rect_h > 0.0f)
			{
				float scale_x = (this->size().width() ) / rect_w;
				float scale_y = (this->size().height()) / rect_h;
				float scale   = (scale_x < scale_y) ? scale_x : scale_y;
				m_zoom_scale = m_zoom_scale * scale;
				b_right_mouse_scal = TRUE;
			}			
			UpdateViewMatrix();
			Draw();
			m_right_mouse_up = false;
			m_right_mouse_down = false;
			m_right_mouse_move = false;
		}
		m_right_mouse_up = true;
		m_right_mouse_place = QCursor::pos();
		loadMenu();
	}
}

void View2D::UpdateViewMatrix()
{
	float view_size_x = this->size().width();
	float view_size_y = this->size().height();
	float view_size = ((view_size_x < view_size_y) ? view_size_x : view_size_y ) - 2 * m_margin ; 
	float m11 =  view_size / m_scene_size_x * m_zoom_scale;
	float m22 = -view_size / m_scene_size_y * m_zoom_scale;
	float dx  = view_size_x / 2.0f - m_scene_offset_x * m11;
	float dy  = view_size_y / 2.0f - m_scene_offset_y * m22;
	m_view_matrix.setMatrix(m11,0.0f,0.0f,m22,dx,dy);
}

void View2D::Draw()
{
	repaint();
}


void View2D::wheelEvent(QWheelEvent* event)
{
	event->accept();
	event->delta() > 0 ? this->ZoomIn() : this->ZoomOut();
}

void View2D::ZoomIn()
{
	m_zoom_scale = m_zoom_scale  * m_zoom_step;
	UpdateViewMatrix();
	Draw();
}

void View2D::ZoomOut()
{
	m_zoom_scale = m_zoom_scale / m_zoom_step;
	UpdateViewMatrix();
	Draw();
}

void View2D::Reset()
{
	m_zoom_scale     = 1.0f;
	m_scene_offset_x = 0.0f;
	m_scene_offset_y = 0.0f;
	b_right_mouse_scal = false;
	UpdateViewMatrix();
	Draw();
}

void View2D::DrawScaleRect()
{
	if ( m_rect_scaling )
	{
		float x = m_scale_rect_left_top.x(); 
		float y = m_scale_rect_left_top.y();
		float w = m_scale_rect_right_bottom.x() - x;
		float h = m_scale_rect_right_bottom.y() - y;
		if (0.0 != w && 0.0 != h)
		{
			m_render->save();
			{
				m_render->setBrush(Qt::transparent);
				m_render->setPen(QPen(QColor(255, 255,255, 255),0.0f,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
				m_render->drawRect(x,y,w,h);
			}
			m_render->restore();
		}	
	}
	//获取系统像素大小
	//显示鼠标所指位置的角度和距离信息
//	if (b_leftbutton_press/*&&!b_right_mouse_scal*/)
//	{
        DispLeftPressInfo();
//	}



}


void View2D::mouseDoubleClickEvent(QMouseEvent * event)
{
	Reset();
}


void View2D::loadMenu()
{
	if (m_right_mouse_down&&m_right_mouse_up&!m_right_mouse_move)
	{
		m_main_menu->exec(m_right_mouse_place);
		m_right_mouse_up = false;
		m_right_mouse_down = false;
		m_right_mouse_move = false;
    }
}

void View2D::initShipPoint()
{
    this->m_centerPoint  = QPointF(this->width()/2.0,this->height()/2.0);

    qDebug()<<this->m_centerPoint;
    m_headShipPoint = QPointF(this->m_centerPoint.x(),this->m_centerPoint.y()-m_height);

    m_headLeftShipPoint = QPointF(m_centerPoint.x() - m_width/2.0, m_centerPoint.y() - (4/5.0*m_height));
    m_headRightShipPoint = QPointF(m_centerPoint.x() + m_width/2.0, m_centerPoint.y() - (4/5.0*m_height));


    m_tailLeftShipPoint = QPointF(m_centerPoint.x() - m_width/2.0, m_centerPoint.y() );
    m_tailRightShipPoint = QPointF(m_centerPoint.x() + m_width/2.0, m_centerPoint.y());

}

void View2D::DrawShip(QPainter *painter)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(155,155,155),2));


//    float m11 =  view_size / m_scene_size_x * m_zoom_scale;
//    float m22 = -view_size / m_scene_size_y * m_zoom_scale;

    QPointF headShipPoint = RotaryAngle(m_centerPoint,m_headShipPoint);
    QPointF headLeftShipPoint = RotaryAngle(m_centerPoint,m_headLeftShipPoint);
    QPointF tailLeftShipPoint = RotaryAngle(m_centerPoint,m_tailLeftShipPoint);
    QPointF tailRightShipPoint = RotaryAngle(m_centerPoint,m_tailRightShipPoint);
    QPointF headRightShipPoint = RotaryAngle(m_centerPoint,m_headRightShipPoint);
    const static QPointF polygon[5] = {headShipPoint,headLeftShipPoint, tailLeftShipPoint,tailRightShipPoint,headRightShipPoint};
    const static QPointF polygon1[5] = {m_headShipPoint,m_headLeftShipPoint, m_tailLeftShipPoint,m_tailRightShipPoint,m_headRightShipPoint};
    painter->drawPolygon(polygon, 5);
     painter->drawPolygon(polygon1, 5);
}

QPointF View2D::RotaryAngle(QPointF centerPt, QPointF lastPt)
{
    //float x'= (x - rx0)*cos(RotaryAngle) + (y - ry0)*sin(RotaryAngle) + rx0 ;
    //float y'=-(x - rx0)*sin(RotaryAngle) + (y - ry0)*cos(RotaryAngle) + ry0 ;
    float angle = 36;
    double angleHude = angle * M_PI / 180;
    float x = (lastPt.x() - centerPt.x())*qCos(angleHude) - (lastPt.y() - centerPt.y())*qSin(angleHude) + centerPt.x();

    float y = (lastPt.x() - centerPt.x())*qSin(angleHude) + (lastPt.y() - centerPt.y())*qCos(angleHude) + centerPt.y();

    return QPointF(x,y);
}

void View2D::clear_track()
{
	Scene2D::GetPointer()->ClearTargetList();
    //TargetStateManager::GetPointer()->clear_target_state_manager();
}

void View2D::clear_plot()
{
	Scene2D::GetPointer()->ClearPlotList();
	DistanceScene2D::GetPointer()->ClearPlotList();
	AngleScene2D::GetPointer()->ClearPlotList();	
	VelocityScene2D::GetPointer()->ClearPlotList();
	DistanceScene2DRadar2::GetPointer()->ClearPlotList();
	AngleScene2DRadar2::GetPointer()->ClearPlotList();
	VelocityScene2DRadar2::GetPointer()->ClearPlotList();
	DistanceScene2DRadar3::GetPointer()->ClearPlotList();
	AngleScene2DRadar3::GetPointer()->ClearPlotList();
	VelocityScene2DRadar3::GetPointer()->ClearPlotList();
}


void View2D::DispLeftPressInfo()
{
	m_render->save();
	{
		QFont font("宋体",10);
		QPainterPath path;
		QPainterPath path1;
		QString str;
		str = QString("%4").arg(m_left_mouse_angle,0,'f',3);
		path.addText(QPointF(m_left_mouse_location.x()+10,m_left_mouse_location.y()-40), font,str); 
		str = QString("%4").arg((int)m_left_mouse_dist);
		path1.addText(QPointF(m_left_mouse_location.x()+10,m_left_mouse_location.y()-20), font,str);

		m_render->setBrush(Qt::transparent);
		m_render->setPen(QPen(QColor(255, 255,255, 255),0.0f,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
		m_render->drawPath(path1);
		m_render->drawPath(path);
		m_render->drawRect(m_left_mouse_location.x()+5,m_left_mouse_location.y() - 55,70,40);
	}
	m_render->restore();
}
