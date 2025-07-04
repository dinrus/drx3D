// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "PropertyRowDrxColor.h"
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <Serialization/PropertyTree/PropertyRowNumber.h>
#include <Serialization/PropertyTree/IMenu.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <QMenu>
#include <QFileDialog>
#include <QPainter>
#include <QColorDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QPushButton>

using Serialization::Vec3AsColor;
typedef SerializableColor_tpl<u8> SerializableColorB;
typedef SerializableColor_tpl<float>         SerializableColorF;

//////////////////////////////////////////////////////////////////////////

QColor ToQColor(const ColorB& v)
{
	return QColor(v.r, v.g, v.b, v.a);
}

void FromQColor(SerializableColorB& vColor, QColor color)
{
	vColor.r = color.red();
	vColor.g = color.green();
	vColor.b = color.blue();
}

QColor ToQColor(const Vec3AsColor& v)
{
	return QColor::fromRgbF(clamp_tpl(v.v.x, 0.0f, 1.0f), clamp_tpl(v.v.y, 0.0f, 1.0f), clamp_tpl(v.v.z, 0.0f, 1.0f));
}

void FromQColor(Vec3AsColor& vColor, QColor color)
{
	ColorF linear(color.redF(), color.greenF(), color.blueF());
	vColor.v.x = linear.r;
	vColor.v.y = linear.g;
	vColor.v.z = linear.b;
}

QColor ToQColor(const SerializableColorF& v)
{
	QColor res = QColor::fromRgbF(clamp_tpl(v.r, 0.0f, 1.0f), clamp_tpl(v.g, 0.0f, 1.0f), clamp_tpl(v.b, 0.0f, 1.0f));
	res.setAlphaF(v.a);
	return res;
}

void FromQColor(SerializableColorF& vColor, QColor color)
{
	ColorF linear(color.redF(), color.greenF(), color.blueF());
	vColor.r = linear.r;
	vColor.g = linear.g;
	vColor.b = linear.b;
	vColor.a = color.alphaF();
}

//////////////////////////////////////////////////////////////////////////
#if 0
template<class ColorClass>
class QPropertyColorDialog : public QColorDialog
{
public:
	QPropertyColorDialog(const QColor& col_, PropertyTree* tree_, PropertyRow* row_)
		: QColorDialog(col_)
		, origcol(col_)
		, tree(tree_)
		, row(row_)
		, beginRowInvoked(false)
	{
		QPropertyTree* ptree = static_cast<QPropertyTree*>(tree);

		QObject::connect(this, &QColorDialog::currentColorChanged,
		                 [this](const QColor& clr)
		{
			PropertyRowDrxColor<ColorClass>* colorRow = static_cast<PropertyRowDrxColor<ColorClass>*>(row);
			if (!colorRow)
				return;

			if (0 == (QApplication::mouseButtons() & Qt::LeftButton) || !beginRowInvoked)
			{
			  beginRowInvoked = true;
			  tree->model()->rowAboutToBeChanged(row);
			}

			colorRow->setColor(clr);

			if (0 == (QApplication::mouseButtons() & Qt::LeftButton) && beginRowInvoked)
			{
			  beginRowInvoked = false;
			  tree->model()->rowChanged(row);
			}
			else
			{
			  tree->apply(true);
			  tree->repaint();
			}
		}
		                 );

		QObject::connect(this, &QColorDialog::finished,
		                 [this](i32 result)
		{
			PropertyRowDrxColor<ColorClass>* colorRow = static_cast<PropertyRowDrxColor<ColorClass>*>(row);
			if (colorRow)
				colorRow->resetPicker();

			if (beginRowInvoked)
			{
			  tree->model()->rowChanged(row);
			  beginRowInvoked = false;
			}

			if (QDialog::Rejected == result)
			{
			  tree->model()->rowAboutToBeChanged(row);
			  colorRow->setColor(origcol);
			  tree->model()->rowChanged(row);
			}
		}
		                 );
	}

	virtual ~QPropertyColorDialog() override
	{
	}

	virtual void mouseReleaseEvent(QMouseEvent* e) override
	{
		if (beginRowInvoked)
		{
			tree->model()->rowChanged(row);
			beginRowInvoked = false;
		}

		__super::mouseReleaseEvent(e);
	}

	const QColor  origcol;
	PropertyTree* tree;
	PropertyRow*  row;
	bool          beginRowInvoked;
};

#endif
//////////////////////////////////////////////////////////////////////////

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::setValueAndContext(const Serialization::SStruct& ser, IArchive& ar)
{
	color_ = ToQColor(*(ColorClass*)ser.pointer());
}

template<class ColorClass>
bool PropertyRowDrxColor<ColorClass >::assignTo(const Serialization::SStruct& ser) const
{
	FromQColor(*((ColorClass*)ser.pointer()), color_);
	return true;
}

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::serializeValue(yasli::Archive& ar)
{
	i32 r, g, b, a;
	QColor color = color_;
	color.getRgb(&r, &g, &b, &a);
	ar(r, "r", "R");
	ar(g, "g", "G");
	ar(b, "b", "B");
	ar(a, "a", "A");

	if (ar.isInput())
	{
		color.setRgb(r, g, b, a);
		setColor(color);
	}
}

template<class ColorClass>
bool PropertyRowDrxColor<ColorClass >::onActivate(const PropertyActivationEvent& e)
{
	if (e.reason == e.REASON_RELEASE || pPicker)
		return false;

	if (userReadOnly() || userReadOnlyRecurse())
		return false;

#if 0
	const QColor initialColor = getColor();
	QPropertyColorDialog<ColorClass>* picker = new QPropertyColorDialog<ColorClass>(initialColor, e.tree, this);
	picker->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	picker->show();
	pPicker = picker;
#else
	// color picker as pop-up
	const QColor initialColor = getColor();
	QPropertyTree* pTree = (QPropertyTree*)e.tree;

	const QPoint popupCoords = pTree->_toScreen(Point(widgetPos_, pos_.y() + e.tree->_defaultRowHeight()));
	QColorDialog* picker = new QColorDialog(initialColor, pTree);
	picker->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
	picker->setOptions(QColorDialog::NoButtons | QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
	picker->setProperty("colorChanged", false);
	picker->setProperty("finished", false); //workaround for 'pick screen color' in color dialog

	PropertyTree* prepertyTree = e.tree;
	PropertyRowDrxColor<ColorClass>* propertyRow = this;

	QObject::connect(picker, &QColorDialog::currentColorChanged,
	                 [prepertyTree, propertyRow, picker](const QColor& clr)
	{
		QVariant finished = picker->property("finished");
		if (finished.toBool())
			return;

		QVariant colorchanged = picker->property("colorChanged");

		if (!colorchanged.toBool())
		{
		  prepertyTree->model()->rowAboutToBeChanged(propertyRow);
		  picker->setProperty("colorChanged", true);
		}

		propertyRow->setColor(clr);
		prepertyTree->repaint();
		prepertyTree->apply(true);
	}
	                 );

	QObject::connect(picker, &QColorDialog::finished,
	                 [picker, prepertyTree, propertyRow, initialColor, this]()
	{
		QVariant colorchanged = picker->property("colorChanged");
		if (DrxGetAsyncKeyState(VK_ESCAPE))
		{
			picker->setCurrentColor(initialColor);
		}

		if (colorchanged.toBool())
			prepertyTree->model()->rowChanged(propertyRow);

		picker->setProperty("finished", true);
	}
	                 );

	{
		//adjust pos
		const QDesktopWidget* desktop = QApplication::desktop();
		i32k screen = desktop->screenNumber(pTree);
		const QRect screenRect = QApplication::desktop()->screenGeometry(screen);
		const QSize pickerSizeHint = picker->sizeHint();

		QPoint pickerPos(popupCoords.x() - 400, popupCoords.y());

		if (pickerPos.y() + pickerSizeHint.height() > screenRect.bottom())
		{
			pickerPos.setY(popupCoords.y() - e.tree->_defaultRowHeight() - pickerSizeHint.height() - 1);
		}

		if (pickerPos.x() < screenRect.left())
		{
			pickerPos.setX(screenRect.left() + 1);
		}

		if (pickerPos.x() + pickerSizeHint.width() > screenRect.right())
		{
			pickerPos.setX(screenRect.right() - pickerSizeHint.width() - 1);
		}

		picker->move(pickerPos);
	}

	picker->show();

#endif
	return true;
}

template<class ColorClass>
yasli::string PropertyRowDrxColor<ColorClass >::valueAsString() const
{
	char buf[64];
	drx_sprintf(buf, "%d %d %d", (i32)color_.red(), (i32)color_.green(), (i32)color_.blue());
	return yasli::string(buf);
}

template<class ColorClass>
bool PropertyRowDrxColor<ColorClass >::onContextMenu(IMenu& menu, PropertyTree* tree)
{
	yasli::SharedPtr<PropertyRowDrxColor> selfPointer(this);
	DrxColorMenuHandler* handler = new DrxColorMenuHandler(tree, this);
	menu.addAction("Pick Color", "", 0, handler, &DrxColorMenuHandler::onMenuPickColor);
	tree->addMenuHandler(handler);
	return true;
}

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::redraw(property_tree::IDrawContext& context)
{
	QColor qcolor = getColor();
	property_tree::Color color(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha());
	context.drawColor(context.widgetRect, color);
}

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::closeNonLeaf(const Serialization::SStruct& ser, Serialization::IArchive& ar)
{
	color_ = ToQColor(*(ColorClass*)ser.pointer());
}

static i32 componentFromRowValue(tukk str, ColorB*)
{
	return clamp_tpl(atoi(str), 0, 255);
}

static i32 componentFromRowValue(tukk str, ColorF*)
{
	return clamp_tpl(i32(atof(str) * 255.0f + 0.5f), 0, 255);
}

static i32 componentFromRowValue(tukk str, Vec3AsColor*)
{
	return clamp_tpl(i32(atof(str) * 255.0f + 0.5f), 0, 255);
}

template<class ColorClass>
PropertyRowDrxColor<ColorClass>::~PropertyRowDrxColor()
{
	if (pPicker)
	{
		pPicker->deleteLater();
		pPicker = nullptr;
	}
}

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::handleChildrenChange()
{
	// generally is not needed unless we are using callbacks
	PropertyRow* rows[4] = {
		childByIndex(0),
		childByIndex(1),
		childByIndex(2),
		childByIndex(3)
	};

	if (rows[0])
		color_.setRed(componentFromRowValue(rows[0]->valueAsString().c_str(), (ColorClass*)0));
	if (rows[1])
		color_.setGreen(componentFromRowValue(rows[1]->valueAsString().c_str(), (ColorClass*)0));
	if (rows[2])
		color_.setBlue(componentFromRowValue(rows[2]->valueAsString().c_str(), (ColorClass*)0));
	if (rows[3])
		color_.setAlpha(componentFromRowValue(rows[3]->valueAsString().c_str(), (ColorClass*)0));
}

template<class ColorClass>
bool PropertyRowDrxColor<ColorClass >::pickColor(PropertyTree* tree)
{
	PropertyActivationEvent e;
	e.reason = PropertyActivationEvent::REASON_PRESS;
	e.tree = tree;
	return onActivate(e);
}

template<typename T>
static void setValueToRowNumberChild(PropertyRow* row, i32 childIndex, T value)
{
	PropertyRow* child = row->childByIndex(childIndex);
	if (child && child->typeId() == Serialization::TypeID::get<T>())
	{
		(static_cast<PropertyRowNumber<T>*>(child))->setVal(value, 0, Serialization::TypeID::get<T>());
	}
}

template<typename T>
static void updateChildren(PropertyRowDrxColor<T>* row)
{
	T linearColor;
	FromQColor(linearColor, row->getColor());
	setValueToRowNumberChild(row, 0, linearColor.r);
	setValueToRowNumberChild(row, 1, linearColor.g);
	setValueToRowNumberChild(row, 2, linearColor.b);
	setValueToRowNumberChild(row, 3, linearColor.a);
}

template<>
static void updateChildren(PropertyRowDrxColor<Vec3AsColor>* row)
{
	Vec3 v;
	Vec3AsColor linearColor(v);
	FromQColor(linearColor, row->getColor());

	setValueToRowNumberChild(row, 0, v.x);
	setValueToRowNumberChild(row, 1, v.y);
	setValueToRowNumberChild(row, 2, v.z);
}

template<class ColorClass>
void PropertyRowDrxColor<ColorClass >::setColor(const QColor& color)
{
	color_.setRed(color.red());
	color_.setGreen(color.green());
	color_.setBlue(color.blue());
	color_.setAlpha(color.alpha());

	updateChildren(this); //manually update color values in row
}

DrxColorMenuHandler::DrxColorMenuHandler(PropertyTree* tree, IPropertyRowDrxColor* propertyRowColor)
	: propertyRowColor(propertyRowColor), tree(tree)
{
}

void DrxColorMenuHandler::onMenuPickColor()
{
	propertyRowColor->pickColor(tree);
}

typedef PropertyRowDrxColor<SerializableColorB> PropertyRowDrxColorB;
typedef PropertyRowDrxColor<Vec3AsColor>        PropertyRowVec3AsColor;
typedef PropertyRowDrxColor<SerializableColorF> PropertyRowDrxColorF;

REGISTER_PROPERTY_ROW(SerializableColorB, PropertyRowDrxColorB);
REGISTER_PROPERTY_ROW(Vec3AsColor, PropertyRowVec3AsColor);
REGISTER_PROPERTY_ROW(SerializableColorF, PropertyRowDrxColorF);

