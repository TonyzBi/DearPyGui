#include "mvColorMapSlider.h"
#include "mvApp.h"
#include <array>
#include "mvItemRegistry.h"
#include "mvPythonExceptions.h"
#include "AppItems/fonts/mvFont.h"
#include "AppItems/themes/mvTheme.h"
#include "AppItems/containers/mvDragPayload.h"

namespace Marvel {

	void mvColorMapSlider::InsertParser(std::map<std::string, mvPythonParser>* parsers)
	{
		mvPythonParser parser(mvPyDataType::UUID, "Adds a color button.", { "Widgets" });
		mvAppItem::AddCommonArgs(parser, (CommonParserArgs)(
			MV_PARSER_ARG_ID |
			MV_PARSER_ARG_WIDTH |
			MV_PARSER_ARG_HEIGHT |
			MV_PARSER_ARG_INDENT |
			MV_PARSER_ARG_PARENT |
			MV_PARSER_ARG_BEFORE |
			MV_PARSER_ARG_CALLBACK |
			MV_PARSER_ARG_SHOW |
			MV_PARSER_ARG_FILTER |
			MV_PARSER_ARG_DROP_CALLBACK |
			MV_PARSER_ARG_DRAG_CALLBACK |
			MV_PARSER_ARG_PAYLOAD_TYPE |
			MV_PARSER_ARG_TRACKED |
			MV_PARSER_ARG_POS)
		);

		parser.addArg<mvPyDataType::Float>("default_value", mvArgType::KEYWORD_ARG, "0.0");

		parser.finalize();

		parsers->insert({ s_command, parser });
	}

	mvColorMapSlider::mvColorMapSlider(mvUUID uuid)
		:
		mvAppItem(uuid)
	{
	}

	void mvColorMapSlider::applySpecificTemplate(mvAppItem* item)
	{
		auto titem = static_cast<mvColorMapSlider*>(item);
		if (_source != 0) _value = titem->_value;
		_color = titem->_color;
		_colormap = titem->_colormap;

	}

	void mvColorMapSlider::draw(ImDrawList* drawlist, float x, float y)
	{

		//-----------------------------------------------------------------------------
		// pre draw
		//-----------------------------------------------------------------------------

		// show/hide
		if (!_show)
			return;

		// focusing
		if (_focusNextFrame)
		{
			ImGui::SetKeyboardFocusHere();
			_focusNextFrame = false;
		}

		// cache old cursor position
		ImVec2 previousCursorPos = ImGui::GetCursorPos();

		// set cursor position if user set
		if (_dirtyPos)
			ImGui::SetCursorPos(_state.getItemPos());

		// update widget's position state
		_state.setPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY() });

		// set item width
		if (_width != 0)
			ImGui::SetNextItemWidth((float)_width);

		// set indent
		if (_indent > 0.0f)
			ImGui::Indent(_indent);

		// push font if a font object is attached
		if (_font)
		{
			ImFont* fontptr = static_cast<mvFont*>(_font.get())->getFontPtr();
			ImGui::PushFont(fontptr);
		}

		// handle enabled theming
		if (_enabled)
		{
			// push class theme (if it exists)
			if (auto classTheme = getClassTheme())
				static_cast<mvTheme*>(classTheme.get())->draw(nullptr, 0.0f, 0.0f);

			// push item theme (if it exists)
			if (_theme)
				static_cast<mvTheme*>(_theme.get())->draw(nullptr, 0.0f, 0.0f);
		}

		// handled disabled theming
		else
		{
			// push class theme (if it exists)
			if (auto classTheme = getClassDisabledTheme())
				static_cast<mvTheme*>(classTheme.get())->draw(nullptr, 0.0f, 0.0f);

			// push item theme (if it exists)
			if (_disabledTheme)
				static_cast<mvTheme*>(_disabledTheme.get())->draw(nullptr, 0.0f, 0.0f);
		}


		//-----------------------------------------------------------------------------
		// draw
		//-----------------------------------------------------------------------------
		{
			ScopedID id(_uuid);

			if (ImPlot::ColormapSlider(_internalLabel.c_str(), _value.get(), &_color, "", _colormap))
			{
				if(_alias.empty())
					mvApp::GetApp()->getCallbackRegistry().submitCallback([=]() {
						mvApp::GetApp()->getCallbackRegistry().addCallback(getCallback(false), _uuid, ToPyFloat(*_value), _user_data);
						});
				else
					mvApp::GetApp()->getCallbackRegistry().submitCallback([=]() {
						mvApp::GetApp()->getCallbackRegistry().addCallback(getCallback(false), _alias, ToPyFloat(*_value), _user_data);
						});
			}
		}

		//-----------------------------------------------------------------------------
		// update state
		//   * only update if applicable
		//-----------------------------------------------------------------------------
		_state._lastFrameUpdate = mvApp::s_frame;
		_state._hovered = ImGui::IsItemHovered();
		_state._active = ImGui::IsItemActive();
		_state._focused = ImGui::IsItemFocused();
		_state._leftclicked = ImGui::IsItemClicked();
		_state._rightclicked = ImGui::IsItemClicked(1);
		_state._middleclicked = ImGui::IsItemClicked(2);
		_state._visible = ImGui::IsItemVisible();
		_state._edited = ImGui::IsItemEdited();
		_state._activated = ImGui::IsItemActivated();
		_state._deactivated = ImGui::IsItemDeactivated();
		_state._deactivatedAfterEdit = ImGui::IsItemDeactivatedAfterEdit();
		_state._rectMin = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y };
		_state._rectMax = { ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y };
		_state._rectSize = { ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y };
		_state._contextRegionAvail = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };

		//-----------------------------------------------------------------------------
		// post draw
		//-----------------------------------------------------------------------------

		// set cursor position to cached position
		if (_dirtyPos)
			ImGui::SetCursorPos(previousCursorPos);

		if (_indent > 0.0f)
			ImGui::Unindent(_indent);

		// pop font off stack
		if (_font)
			ImGui::PopFont();

		// handle popping styles
		if (_enabled)
		{
			if (auto classTheme = getClassTheme())
				static_cast<mvTheme*>(classTheme.get())->customAction();

			if (_theme)
				static_cast<mvTheme*>(_theme.get())->customAction();
		}
		else
		{
			if (auto classTheme = getClassDisabledTheme())
				static_cast<mvTheme*>(classTheme.get())->customAction();

			if (_disabledTheme)
				static_cast<mvTheme*>(_disabledTheme.get())->customAction();
		}

		// handle widget's event handlers
		for (auto& item : _children[3])
		{
			if (!item->preDraw())
				continue;

			item->draw(nullptr, ImGui::GetCursorPosX(), ImGui::GetCursorPosY());
		}

		// handle drag & drop payloads
		for (auto& item : _children[4])
		{
			if (!item->preDraw())
				continue;

			item->draw(nullptr, ImGui::GetCursorPosX(), ImGui::GetCursorPosY());
		}

		// handle drag & drop if used
		if (_dropCallback)
		{
			ScopedID id(_uuid);
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_payloadType.c_str()))
				{
					auto payloadActual = static_cast<const mvDragPayload*>(payload->Data);
					if (_alias.empty())
						mvApp::GetApp()->getCallbackRegistry().addCallback(getDropCallback(), _uuid, payloadActual->getDragData(), nullptr);
					else
						mvApp::GetApp()->getCallbackRegistry().addCallback(getDropCallback(), _alias, payloadActual->getDragData(), nullptr);
				}

				ImGui::EndDragDropTarget();
			}
		}
	}

	PyObject* mvColorMapSlider::getPyValue()
	{
		return ToPyFloat(*_value);
	}

	void mvColorMapSlider::setPyValue(PyObject* value)
	{
		*_value = ToFloat(value);
	}

	void mvColorMapSlider::setDataSource(mvUUID dataSource)
	{
		if (dataSource == _source) return;
		_source = dataSource;

		mvAppItem* item = mvApp::GetApp()->getItemRegistry().getItem(dataSource);
		if (!item)
		{
			mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
				"Source item not found: " + std::to_string(dataSource), this);
			return;
		}
		if (item->getValueType() != getValueType())
		{
			mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
				"Values types do not match: " + std::to_string(dataSource), this);
			return;
		}
		_value = *static_cast<std::shared_ptr<float>*>(item->getValue());
	}

}