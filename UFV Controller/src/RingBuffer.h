#pragma once

template<typename T>
class RingBuffer
{
public:
	int m_head = 0;
	int m_tail = 0;
	int m_size = 0;
	T* m_data = nullptr;
public:
	RingBuffer();
	RingBuffer(int size);
	~RingBuffer();
	void Write(const T& value);
	void Write(const T&& value);
};

template<typename T>
RingBuffer<T>::RingBuffer() : RingBuffer(16)
{
}

template<typename T>
RingBuffer<T>::RingBuffer(int size)
{
	m_size = size;
	m_data = new T[size];
}

template<typename T>
RingBuffer<T>::~RingBuffer()
{
	delete[] m_data;
	m_data = nullptr;
}

template<typename T>
void RingBuffer<T>::Write(const T& value)
{
	m_data[head] = value;
	m_head = (m_head + 1) % RINGBUF_SIZE;
	if (m_tail == m_head) // full
		m_tail = (m_tail + 1) % RINGBUF_SIZE;
}

template<typename T>
void RingBuffer<T>::Write(const T&& value)
{
	m_data[m_head] = value;
	m_head = (m_head + 1) % m_size;
	if (m_tail == m_head) // full
		m_tail = (m_tail + 1) % m_size;
}