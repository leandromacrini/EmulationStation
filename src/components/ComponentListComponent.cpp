#include "ComponentListComponent.h"
#include "../Log.h"
#include "../Renderer.h"

#define INITIAL_CELL_SIZE 12

ComponentListComponent::ComponentListComponent(Window* window, Eigen::Vector2i gridDimensions) : GuiComponent(window), 
	mGrid(NULL), mColumnWidths(NULL), mRowHeights(NULL), 
	mColumnWidthForced(NULL), mRowHeightForced(NULL), 
	mCursor(-1, -1)
{
	mEntries.reserve(gridDimensions.x() * gridDimensions.y());
	makeCells(gridDimensions);
}

void ComponentListComponent::makeCells(Eigen::Vector2i size)
{
	if(mGrid)
		delete[] mGrid;
	if(mColumnWidths)
		delete[] mColumnWidths;
	if(mRowHeights)
		delete[] mRowHeights;
	
	mGridSize = size;
	mGrid = new ComponentEntry*[size.x() * size.y()];
	std::fill(mGrid, mGrid + (size.x() * size.y()), (ComponentEntry*)NULL);

	mColumnWidths = new unsigned int[size.x()];
	std::fill(mColumnWidths, mColumnWidths + size.x(), INITIAL_CELL_SIZE);
	
	mRowHeights = new unsigned int[size.y()];
	std::fill(mRowHeights, mRowHeights + size.y(), INITIAL_CELL_SIZE);

	mColumnWidthForced = new bool[size.x()];
	std::fill(mColumnWidthForced, mColumnWidthForced + size.x(), false);

	mRowHeightForced = new bool[size.y()];
	std::fill(mRowHeightForced, mRowHeightForced + size.y(), false);

	updateSize();
	resetCursor();
}

void ComponentListComponent::setEntry(Eigen::Vector2i pos, Eigen::Vector2i size, GuiComponent* component, bool canFocus, AlignmentType align, 
	Eigen::Matrix<bool, 1, 2> autoFit, UpdateBehavior updateType)
{
	if(pos.x() > mGridSize.x() || pos.y() > mGridSize.y() || pos.x() < 0 || pos.y() < 0)
	{
		LOG(LogError) << "Tried to set entry beyond grid size!";
		return;
	}

	if(component == NULL)
	{
		LOG(LogError) << "Tried to add NULL component to ComponentList!";
		return;
	}

	ComponentEntry entry(Eigen::Vector2i(pos.x(), pos.y()), Eigen::Vector2i(size.x(), size.y()), component, updateType, canFocus, align);
	
	mEntries.push_back(entry);

	for(int y = pos.y(); y < pos.y() + size.y(); y++)
	{
		for(int x = pos.x(); x < pos.x() + size.x(); x++)
		{
			setCell(x, y, &mEntries.back());
		}
	}

	if(component->getParent() != NULL)
		LOG(LogError) << "ComponentListComponent ruining an existing parent-child relationship! Call a social worker!";
	component->setParent(this);

	if(!cursorValid() && canFocus)
		mCursor = pos;

	//update the column width and row height
	//if(autoFit.x() && (int)getColumnWidth(pos.x()) < component->getSize().x())
	//	setColumnWidth(pos.x(), (unsigned int)component->getSize().x());
	//if(autoFit.y() && (int)getRowHeight(pos.y()) < component->getSize().y())
	//	setRowHeight(pos.y(), (unsigned int)component->getSize().y());
	updateCellSize(&mEntries.back(), autoFit.x(), autoFit.y());

	component->setPosition(getCellOffset(pos));

	updateSize();
}

void ComponentListComponent::forceRowHeight(int row, unsigned int size)
{
	mRowHeights[row] = size;
	mRowHeightForced[row] = true;
	updateSize();
	updateComponentOffsets();
}

void ComponentListComponent::forceColumnWidth(int col, unsigned int size)
{
	mColumnWidths[col] = size;
	mRowHeightForced[col] = true;
	updateSize();
	updateComponentOffsets();
}

unsigned int ComponentListComponent::getRowHeight(int row) { return mRowHeights[row]; }
unsigned int ComponentListComponent::getColumnWidth(int col) { return mColumnWidths[col]; }

Eigen::Vector3f ComponentListComponent::getCellOffset(Eigen::Vector2i pos)
{
	Eigen::Vector3f offset(0, 0, 0);

	for(int y = 0; y < pos.y(); y++)
		offset[1] += getRowHeight(y);
	
	for(int x = 0; x < pos.x(); x++)
		offset[0] += getColumnWidth(x);

	ComponentEntry* entry = getCell(pos.x(), pos.y());

	Eigen::Vector2i gridSize(0, 0);
	for(int x = pos.x(); x < pos.x() + entry->dim[0]; x++)
		gridSize[0] += getColumnWidth(x);
	for(int y = pos.y(); y < pos.y() + entry->dim[1]; y++)
		gridSize[1] += getRowHeight(y);

	//if AlignCenter, add half of cell width - half of control width
	if(entry->alignment == AlignCenter)
		offset[0] += gridSize.x() / 2 - entry->component->getSize().x() / 2;

	//if AlignRight, add cell width - control width
	if(entry->alignment == AlignRight)
		offset[0] += gridSize.x() - entry->component->getSize().x();

	//always center on the Y axis
	offset[1] += gridSize.y() / 2.0f - entry->component->getSize().y() / 2.0f;

	return offset;
}

void ComponentListComponent::setCell(unsigned int x, unsigned int y, ComponentEntry* entry)
{
	if(x >= (unsigned int)mGridSize.x() || y >= (unsigned int)mGridSize.y())
	{
		LOG(LogError) << "Invalid setCell - position " << x << ", " << y << " out of bounds!";
		return;
	}

	mGrid[y * mGridSize.x() + x] = entry;
}

ComponentListComponent::ComponentEntry* ComponentListComponent::getCell(unsigned int x, unsigned int y)
{
	if(x >= (unsigned int)mGridSize.x() || y >= (unsigned int)mGridSize.y())
	{
		LOG(LogError) << "Invalid getCell - position " << x << ", " << y << " out of bounds!";
		return NULL;
	}

	return mGrid[y * mGridSize.x() + x];
}

void ComponentListComponent::updateSize()
{
	mSize = Eigen::Vector2f(0, 0);
	for(int x = 0; x < mGridSize.x(); x++)
		mSize.x() += getColumnWidth(x);
	for(int y = 0; y < mGridSize.y(); y++)
		mSize.y() += getRowHeight(y);
}

void ComponentListComponent::updateComponentOffsets()
{
	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		iter->component->setPosition(getCellOffset(iter->pos));
	}
}

void ComponentListComponent::updateCellSize(ComponentEntry* e, bool updWidth, bool updHeight)
{
	if(!e)
	{
		LOG(LogError) << "Tried to updateCellSize NULL ComponentEntry!";
		return;
	}

	unsigned int x = e->pos.x();
	unsigned int y = e->pos.y();

	if(!mColumnWidthForced[x] && updWidth)
	{
		//recalc width to widest in column
		float widest = 0;
		for(int row = 0; row < mGridSize.y(); row++)
		{
			ComponentEntry* check = getCell(x, row);
			if(check)
			{
				if(check->component->getSize().x() > widest)
					widest = check->component->getSize().x();
			}
		}

		mColumnWidths[x] = (unsigned int)widest;
	}
	if(!mRowHeightForced[y] && updHeight)
	{
		float tallest = 0;
		for(int col = 0; col < mGridSize.x(); col++)
		{
			ComponentEntry* check = getCell(col, y);
			if(check)
			{
				if(check->component->getSize().y() > tallest)
					tallest = check->component->getSize().y();
			}
		}

		mRowHeights[y] = (unsigned int)tallest;
	}

	updateComponentOffsets();
	updateSize();
}

void ComponentListComponent::updateComponent(GuiComponent* cmp)
{
	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		if(iter->component == cmp)
		{
			updateCellSize(&(*iter));
		}
	}
}

bool ComponentListComponent::input(InputConfig* config, Input input)
{
	if(cursorValid() && getCell(mCursor.x(), mCursor.y())->component->input(config, input))
		return true;

	if(!input.value)
		return false;

	if(config->isMappedTo("down", input))
	{
		moveCursor(Eigen::Vector2i(0, 1));
		return true;
	}
	if(config->isMappedTo("up", input))
	{
		moveCursor(Eigen::Vector2i(0, -1));
		return true;
	}
	if(config->isMappedTo("left", input))
	{
		moveCursor(Eigen::Vector2i(-1, 0));
		return true;
	}
	if(config->isMappedTo("right", input))
	{
		moveCursor(Eigen::Vector2i(1, 0));
		return true;
	}

	return false;
}

void ComponentListComponent::resetCursor()
{
	if(mEntries.size() == 0)
	{
		mCursor = Eigen::Vector2i(-1, -1);
		return;
	}

	const Eigen::Vector2i origCursor = mCursor;
	mCursor << mEntries.at(0).pos[0], mEntries.at(0).pos[1];
	onCursorMoved(origCursor, mCursor);
}

void ComponentListComponent::moveCursor(Eigen::Vector2i dir)
{
	if(dir.x() != 0 && dir.y() != 0)
	{
		LOG(LogError) << "Invalid cursor move dir!";
		return;
	}

	Eigen::Vector2i origCursor = mCursor;

	if(!cursorValid())
	{
		resetCursor();

		if(!cursorValid())
		{
			if(mCursor != origCursor)
				onCursorMoved(origCursor, mCursor);

			return;
		}
	}

	Eigen::Vector2i searchAxis(dir.x() == 0, dir.y() == 0);
	
	while(mCursor.x() >= 0 && mCursor.y() >= 0 && mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y())
	{
		mCursor = mCursor + dir;

		Eigen::Vector2i curDirPos = mCursor;

		//spread out on search axis+
		while(mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y())
		{
			if(cursorValid() && getCell(mCursor.x(), mCursor.y())->canFocus)
			{
				onCursorMoved(origCursor, mCursor);
				return;
			}

			mCursor += searchAxis;
		}

		//now again on search axis-
		mCursor = curDirPos;
		while(mCursor.x() >= 0 && mCursor.y() >= 0)
		{
			if(cursorValid() && getCell(mCursor.x(), mCursor.y())->canFocus)
			{
				onCursorMoved(origCursor, mCursor);
				return;
			}

			mCursor -= searchAxis;
		}

		mCursor = curDirPos;
	}

	//failed to find another focusable element in this direction
	mCursor = origCursor;
}

bool ComponentListComponent::cursorValid()
{
	if(mCursor.x() < 0 || mCursor.y() < 0 || mCursor.x() >= mGridSize.x() || mCursor.y() >= mGridSize.y())
		return false;

	return getCell(mCursor.x(), mCursor.y()) != NULL;
}

void ComponentListComponent::update(int deltaTime)
{
	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		switch(iter->updateType)
		{
		case UpdateAlways:
			iter->component->update(deltaTime);
			break;

		case UpdateFocused:
			if(cursorValid() && getCell(mCursor.x(), mCursor.y())->component == iter->component)
				iter->component->update(deltaTime);
			break;
		}
	}
}

void ComponentListComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	Renderer::drawRect(0, 0, (int)getSize().x(), (int)getSize().y(), 0xFFFFFFAA);

	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		iter->component->render(trans);
	}

	
	//draw cell outlines
	/*Renderer::setMatrix(trans);
	Eigen::Vector2i pos(0, 0);
	for(int x = 0; x < mGridSize.x(); x++)
	{
		for(int y = 0; y < mGridSize.y(); y++)
		{
			Renderer::drawRect(pos.x(), pos.y(), getColumnWidth(x), 2, 0x000000AA);
			Renderer::drawRect(pos.x(), pos.y(), 2, getRowHeight(y), 0x000000AA);
			Renderer::drawRect(pos.x() + getColumnWidth(x), pos.y(), 2, getRowHeight(y), 0x000000AA);
			Renderer::drawRect(pos.x(), pos.y() + getRowHeight(y) - 2, getColumnWidth(x), 2, 0x000000AA);

			pos[1] += getRowHeight(y);
		}

		pos[1] = 0;
		pos[0] += getColumnWidth(x);
	}*/

	//draw cursor
	if(cursorValid())
	{
		ComponentEntry* entry = getCell(mCursor.x(), mCursor.y());
		Eigen::Affine3f entryTrans = trans * entry->component->getTransform();
		Renderer::setMatrix(entryTrans);

		Renderer::drawRect(0, 0, 4, 4, 0xFF0000FF);
		Renderer::drawRect(0, 0, (int)entry->component->getSize().x(), (int)entry->component->getSize().y(), 0x0000AA22);
	}
}

void ComponentListComponent::textInput(const char* text)
{
	if(getSelectedComponent() != NULL)
		getSelectedComponent()->textInput(text);
}

void ComponentListComponent::onPositionChanged()
{
	updateComponentOffsets();
}

GuiComponent* ComponentListComponent::getSelectedComponent()
{
	if(!cursorValid())
		return NULL;
	return getCell(mCursor.x(), mCursor.y())->component;
}

void ComponentListComponent::onCursorMoved(Eigen::Vector2i from, Eigen::Vector2i to)
{
	if(from != Eigen::Vector2i(-1, -1))
		getCell(from.x(), from.y())->component->onFocusLost();

	if(to != Eigen::Vector2i(-1, -1))
		getCell(to.x(), to.y())->component->onFocusGained();
}
