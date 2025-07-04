// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <memory>
#include <QWidget>

#include <drx3D/CoreX/Serialization/Forward.h>

struct BlockPaletteContent;
struct BlockPaletteLayout;
struct BlockPaletteItem;

struct BlockPaletteSelectedId
{
	union
	{
		long long userId;
		uk     userPointer;
	};

	BlockPaletteSelectedId()
		: userId(0)
	{
	}
};
typedef std::vector<BlockPaletteSelectedId> BlockPaletteSelectedIds;

class BlockPalette : public QWidget
{
	Q_OBJECT
public:
	BlockPalette(QWidget* parent);
	~BlockPalette();

	void                       SetContent(const BlockPaletteContent& content);
	void                       SetAddEventWithSingleClick(bool addWithSingleClick);
	const BlockPaletteContent& Content() const { return *m_content.get(); }

	void                       GetSelectedIds(BlockPaletteSelectedIds* ids) const;
	const BlockPaletteItem*    GetItemByHotkey(i32 hotkey) const;
	void                       Serialize(Serialization::IArchive& ar);
signals:
	void                       SignalChanged();
	void                       SignalSelectionChanged();
	void                       SignalItemClicked(const BlockPaletteItem& item);
protected slots:
	void                       OnMenuAdd();
	void                       OnMenuDelete();
	void                       OnMenuAddWithSingleClick();
	void                       OnMenuAssignHotkey();
private:
	struct MouseHandler;
	struct SelectionHandler;
	struct DragHandler;

	void UpdateLayout();
	void ItemClicked(i32 mouseButton);
	void AssignHotkey(i32 hotkey);

	void paintEvent(QPaintEvent* ev) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void mouseDoubleClickEvent(QMouseEvent* ev) override;
	void keyPressEvent(QKeyEvent* ev) override;
	void resizeEvent(QResizeEvent* ev) override;
	void wheelEvent(QWheelEvent* ev) override;

	std::unique_ptr<BlockPaletteContent> m_content;
	std::unique_ptr<BlockPaletteLayout>  m_layout;
	std::unique_ptr<MouseHandler>        m_mouseHandler;

	std::vector<i32>                     m_selectedItems;
	std::vector<i32>                     m_draggedItems;
	std::vector<i32>                     m_hotkeys;
	bool m_addWithSingleClick;
};

