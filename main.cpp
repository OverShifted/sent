#include <iostream>
#include <chrono>

#include "scene.h"

struct TransformComponent
{
	float pos[3];//, rot[3], scale[3], rotq[4], lmat[16], gmat[16];
};

struct PhysicsVelocityComponent
{
	float v[3];
};

struct PhysicsAccelerationComponent
{
	float a[3];
};

struct PhysicsForceComponent
{
	float f[3];
};

struct PhysicsMassComponent
{
	float m;
};

float rand_float()
{
	return (float)rand() / (float)RAND_MAX;
}

int main()
{
	// For deterministic result
	srand(10);

	uint32_t entity_count;
	std::cin >> entity_count;

	auto t0 = std::chrono::high_resolution_clock::now();
	Scene scene(entity_count);

	for (uint32_t i = 0; i < entity_count; i++)
	{
		auto entity = scene.new_entity();
		scene.add_component<TransformComponent>(entity);

		if (rand_float() > 0.5)
		{
			scene.add_component<PhysicsVelocityComponent>(entity);
			if (rand_float() > 0.5)
			{
				// std::cout << "FMA";
				scene.add_component<PhysicsForceComponent>(entity);
				scene.add_component<PhysicsAccelerationComponent>(entity);
				scene.add_component<PhysicsMassComponent>(entity);
			}
		}
	}

	std::chrono::duration<float> d = std::chrono::high_resolution_clock::now() - t0;
	std::cout << "Entity creation took " << d.count() * 1000 << "ms\n";

	t0 = std::chrono::high_resolution_clock::now();

	// a = ΣF / m
	{
		ComponentTypeID query[] = {
			component_type_id<PhysicsForceComponent>(),
			component_type_id<PhysicsMassComponent>(),
			component_type_id<PhysicsAccelerationComponent>()
		};

		scene.query([&](Entity id) {
			auto force = scene.get_component<PhysicsForceComponent>(id);
			auto mass = scene.get_component<PhysicsMassComponent>(id);
			auto accel = scene.get_component<PhysicsAccelerationComponent>(id);

			for (int i = 0; i < 3; i++)
				accel->a[i] = force->f[i] / mass->m;
		}, query, query + 3);
	}

	// v += a.t
	{
		ComponentTypeID query[] = {
			component_type_id<PhysicsAccelerationComponent>(),
			component_type_id<PhysicsVelocityComponent>()
		};

		scene.query([&](Entity id) {
			auto accel = scene.get_component<PhysicsAccelerationComponent>(id);
			auto vel = scene.get_component<PhysicsVelocityComponent>(id);

			for (int i = 0; i < 3; i++)
				vel->v[i] = accel->a[i] * 1.3315;
		}, query, query + 2);
	}

	// p += v.t
	{
		ComponentTypeID query[] = {
			component_type_id<PhysicsVelocityComponent>(),
			component_type_id<TransformComponent>()
		};

		scene.query([&](Entity id) {
			auto vel = scene.get_component<PhysicsVelocityComponent>(id);
			auto trans = scene.get_component<TransformComponent>(id);

			for (int i = 0; i < 3; i++)
				trans->pos[i] = vel->v[i] * 6.12346;
		}, query, query + 2);
	}

	d = std::chrono::high_resolution_clock::now() - t0;
	std::cout << "Queries took " << d.count() * 1000 << "ms\n";
}
