#pragma once

#include <vector>
#include <SDL.h>

template <class T>
class ObjectPool
{
public:

	void Create(unsigned int num_objects)
	{
		Deallocate();

		for (unsigned int i = 0; i < num_objects; i++)
		{
			T * t = new T();
			pool.push_back(t);
		}
	}

	void Destroy()
	{
		for (auto it = pool.begin(); it != pool.end(); it++)
			(*it)->Destroy();
	}

	void Deallocate()
	{
		for (auto it = pool.begin(); it != pool.end(); it++)
			delete *it;
	}

	~ObjectPool()
	{
		Deallocate();
	}

	T* FirstAvailable()
	{
		for (auto it = pool.begin(); it != pool.end(); it++)
			if (!(**it).IsEnabled())
				return (*it);

		// if it reaches this point, there is no available object in the pool
		return NULL;
	}

	/**
	 * Tries to return the first n available objects in the pool,
	 * it may return less.
	 * @param n
	 * @return n or less not enabled objects
	 */
	std::vector<T*> FirstAvailableN(const int n)
    {
	    std::vector<T*> available;
        for (auto it = pool.begin(); it != pool.end() && available.size() < n; it++)
            if (!(**it).IsEnabled())
                available.push_back(*it);
        return available;
    }

	// select a random, enabled element in the object pool
	T* SelectRandom()
	{
		int offset = (rand() / (float)RAND_MAX) * pool.size();

		for (int i = 0; i < pool.size(); i++)
		{
			int index = (i + offset) % pool.size();

			if (pool[index]->enabled)
				return pool[index];
		}

		return NULL;
	}

	std::vector<T*> pool;
private:
};