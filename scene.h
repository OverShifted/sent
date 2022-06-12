#pragma once
#include <vector>
#include <memory>

#include "sparse_set.h"

using ComponentIDType = uint32_t;
using EntityIDType = uint32_t;

ComponentIDType next_component_id()
{
	static ComponentIDType id = 0;
	return id++;
}

template<typename C>
ComponentIDType component_type_id()
{
	static ComponentIDType id = next_component_id();
	return id;
}

class Scene
{
public:
	Scene(uint32_t max_entity_count)
		: m_max_entity_count(max_entity_count)
	{
	}

	EntityIDType new_entity()
	{
		return m_entity_count++;
	}

	template<typename C>
	C* add_component(const EntityIDType& entity)
	{
		if (component_type_id<C>() >= m_data.size())
		{
			m_data.push_back(std::make_shared<ComponentData<C>>(m_max_entity_count));
		}

		auto data = (ComponentData<C>*)m_data[component_type_id<C>()].get();
		data->set.add(entity);

		return get_component<C>(entity);
	}

	template<typename C>
	C* get_component(const EntityIDType& entity)
	{
		if (component_type_id<C>() >= m_data.size())
			return nullptr;

		auto data = (ComponentData<C>*)m_data[component_type_id<C>()].get();
		auto idx = data->set.m_sparse[entity];
		return &data->components[idx];
	}

	template<typename C>
	bool has_component(const EntityIDType& entity)
	{
		return component_type_id<C>() < m_data.size() && m_data[component_type_id<C>()]->set.has(entity);
	}

	bool has_component(const EntityIDType& entity, const ComponentIDType& component)
	{
		return component < m_data.size() && m_data[component]->set.has(entity);
	}

	template<typename C>
	void remove_component(const EntityIDType& entity)
	{
		if (component_type_id<C>() >= m_data.size())
			return;

		auto data = (ComponentData<C>*)m_data[component_type_id<C>()].get();
		auto idx = data->set.m_sparse[entity];
		data->components[idx] = data->components[data->set.m_size];
		data->set.remove(entity);
	}

	template<typename F>
	void query(F fn, ComponentIDType* begin, ComponentIDType* end)
	{
		uint32_t min_size = UINT32_MAX;
		SparseSet<EntityIDType, uint32_t>* min_set;

		for (ComponentIDType* type_id = begin; type_id < end; type_id++)
		{
			if (m_data.size() <= *type_id)
			{
				std::cout << "Returned! " << m_data.size() << " >= " << *type_id << "\n";
				return;
			}

			auto data = m_data[*type_id].get();
			if (data->set.m_size < min_size)
			{
				min_set = &data->set;
			}
		}

		// Loop over entities with that specific component
		for (uint32_t i = 0; i < min_set->m_size; i++)
		{
			auto entity = min_set->m_dense[i];

			// Make sure it has other components as well
			bool has_all = true;
			for (ComponentIDType* type_id = begin; type_id < end; type_id++)
			{
				if (!has_component(entity, *type_id))
				{
					has_all = false;
					break;
				}
			}

			if (has_all)
				fn(entity);
		}
	}
private:
	struct IComponentData
	{
		IComponentData(uint32_t max_entity_count)
			: set(max_entity_count)
		{
		}

		SparseSet<EntityIDType, uint32_t> set;
	};

	template<typename C>
	struct ComponentData : public IComponentData
	{
		ComponentData(uint32_t max_entity_count)
			: IComponentData(max_entity_count)
		{
			components.resize(max_entity_count);
		}

		std::vector<C> components;
	};

	std::vector<std::shared_ptr<IComponentData>> m_data;

	EntityIDType m_entity_count = 0;
	uint32_t m_max_entity_count;
};
