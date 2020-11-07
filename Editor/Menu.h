#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>

namespace Columbus::Editor
{

	struct MenuItem
	{
	private:
		std::string title;
		std::string shortcut;
		std::unordered_map<std::string, MenuItem> children;
	public:
		std::function<void()> onExecute;
		bool* selected = nullptr;

		MenuItem() {}
		MenuItem(std::string_view title) : title(title) {}
		MenuItem(std::string_view title, std::function<void()> onExecute, std::unordered_map<std::string, MenuItem>&& children) :
			title(title), onExecute(onExecute), children(std::move(children)) {}

		void Draw();

		MenuItem& Shortcut(std::string_view scut)
		{
			this->shortcut = scut;
			return *this;
		}

		MenuItem& FindOrCreate(const std::string& id)
		{
			if (children.find(id) == children.end())
				children[id] = { id };

			return children[id];
		}

		MenuItem& operator[](const std::string& id)
		{
			return children[id];
		}
	};

	struct MenuBar
	{
	public:
		using Menus = std::unordered_map<std::string, MenuItem>;
	private:
		Menus menus;
	public:
		MenuBar() {}
		MenuBar(Menus&& menus) : menus(std::move(menus)) {}

		void Draw();

		MenuItem& FindOrCreate(const std::string& id)
		{
			if (menus.find(id) == menus.end())
				menus[id] = {};

			return menus[id];
		}

		MenuItem& operator[](const std::string& id)
		{
			return menus[id];
		}
	};

}
