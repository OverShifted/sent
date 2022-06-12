#pragma once
#include <vector>

class Scene;

template<typename T, typename I> 
class SparseSet
{
public:
	SparseSet(I capacity)
	{
		m_dense.resize(capacity);
		m_sparse.resize(capacity);
	}

	void add(T v);
	void remove(T v);
	bool has(T v);
private:
	I m_size = 0;
	std::vector<T> m_dense;
	std::vector<I> m_sparse;

	friend class Scene;
};

template<typename T, typename I>
void SparseSet<T, I>::add(T v)
{
	if (has(v))
		return;

	m_dense[m_size] = v;
	m_sparse[v] = m_size;
	m_size++;
}

template<typename T, typename I>
void SparseSet<T, I>::remove(T v)
{
	if (!has(v))
		return;

	I idx = m_sparse[v];
	m_size--;
	T last_value = m_dense[m_size];
	m_dense[idx] = last_value;
	m_sparse[last_value] = idx;
}

template<typename T, typename I>
bool SparseSet<T, I>::has(T v)
{
	I idx = m_sparse[v];
	return idx < m_size && m_dense[idx] == v;
}

