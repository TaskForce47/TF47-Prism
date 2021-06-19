#pragma once

#include "intercept.hpp"

namespace tf47::prism::helper
{
	static r_string get_object_type(object game_object)
	{
		intercept::client::invoker_lock lock;
		game_value object_type_return;
		__SQF(
			(_this call BIS_fnc_objectType) select 1;
		).capture(game_object, object_type_return);
		return object_type_return;
	}

	static r_string get_display_name_vehicle(object& vehicle)
	{
		r_string vehicleName = get_text(
			intercept::sqf::config_entry(intercept::sqf::config_file()) >> "CfgVehicles" >>
			intercept::sqf::type_of(vehicle) >> "displayName");
		return vehicleName;
	}

	template<typename T, typename Y>
	class thread_safe_vector
	{
	private:
		std::vector<std::pair<T, Y>> data_vector_;
		std::recursive_mutex lock_;
	public:
		thread_safe_vector() {}

		Y* get(T key)
		{
			std::lock_guard guard(lock_);
			if (data_vector_.size() == 0) return nullptr;

			for (size_t i = 0; i < data_vector_.size(); i++)
			{
				if (data_vector_[i].first == key)
				{
					return &(data_vector_[i].second);
				}
			}
			
			return nullptr;
		}
		void insert(std::pair<T, Y> data, bool overwrite = true)
		{
			std::lock_guard guard(lock_);
			
			for (auto element : data_vector_)
			{
				if (element.first == data.first)
				{
					if (overwrite)
					{
						element.second = data.second;
						return;
					}
					else
					{
						return;
					}
				}
			}
				
			data_vector_.push_back(data);
		}
		bool exist(T key)
		{
			std::lock_guard guard(lock_);

			for (auto element : data_vector_)
			{
				if (element.first == key)
				{
					return true;
				}
			}
			
			return false;
		}
		void remove(T key)
		{
			std::lock_guard guard(lock_);

			for (auto element : data_vector_)
			{
				if (element.first == key)
				{
					data_vector_.erase(element);
					return;
				}
			}
		}
		void clear()
		{
			std::lock_guard guard(lock_);

			data_vector_.clear();
		}
		std::vector<std::pair<T, Y>> get_vector()
		{
			std::lock_guard guard(lock_);

			return data_vector_;
		}
	};
}
