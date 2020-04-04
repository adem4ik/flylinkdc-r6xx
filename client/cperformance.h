#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <fstream>
#include <string>

namespace performance
{
/// ����� ������ �������.
class timer
{
		LARGE_INTEGER m_start;
		
	public:
	
		void start()
		{
			QueryPerformanceCounter(&m_start);
		}
		
		double finish()
		{
			LARGE_INTEGER finish;
			QueryPerformanceCounter(&finish);
			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);
			return double(finish.QuadPart - m_start.QuadPart) / double(freq.QuadPart);
		}
		
		template <class TStream> void finish_output(TStream & out)
		{
			const double l_finish = finish();
			out << l_finish << std::endl;
#ifdef USE_FLY_CONSOLE_TEST
			std::cout << l_finish << std::endl;
#endif
		}
};

/// ����� ������ ������������������ ������������� ���������� ������ ����� ����.
/// ����������� ���������� �� ������� �� ���������� � ���������� ����������.
class instruction_checker
{
		timer m_timer;
		
		class instruction_info
		{
				double m_time;
				size_t m_count;
				
			public:
			
				instruction_info() : m_time(0.0), m_count(0) {}
				void add_time(double time)
				{
					m_time += time;
				}
				void inc_count()
				{
					++m_count;
				}
				
				double get_time() const
				{
					return m_time;
				}
				size_t get_count() const
				{
					return m_count;
				}
				double get_average_time() const
				{
					return m_time / double(m_count);
				}
		} m_info;
		
	public:
	
		void start()
		{
			m_timer.start();
		}
		void finish()
		{
			m_info.add_time(m_timer.finish());
			m_info.inc_count();
		}
		
		double get_time() const
		{
			return m_info.get_time();
		}
		size_t get_count() const
		{
			return m_info.get_count();
		}
		double get_average_time() const
		{
			return m_info.get_average_time();
		}
};

/// ����� - guard ��� ������ �����������-����������� ������� ����.
/// ����� ��������� ������ performance_instruction_checker � �������������
/// � ����������� ������� � ����� ����������� ����������.
/// ������ �������������� ��� ����������� ��������� � �������.
class stream_guard
{
		instruction_checker m_checker;
		int m_instructionNumber;
		std::ofstream & m_outFile;
	public:
		stream_guard(int instructionNumber, std::ofstream & outFile)
			: m_instructionNumber(instructionNumber), m_outFile(outFile)
		{
		}
		void start()
		{
			m_checker.start();
		}
		void finish()
		{
			m_checker.finish();
		}
		~stream_guard()
		{
#ifdef USE_FLY_CONSOLE_TEST
			std::cout << "Instruction number = " << m_instructionNumber << std::endl;
			std::cout << "Hole time = " << m_checker.get_time() << std::endl;
			std::cout << "Call count = " << static_cast<unsigned int>(m_checker.get_count()) << std::endl;
			std::cout << "Average time = " << m_checker.get_average_time() << std::endl;
			std::cout << "===============================================" << std::endl;
#endif
			m_outFile << "Instruction number = " << m_instructionNumber << std::endl;
			m_outFile << "Hole time = " << m_checker.get_time() << std::endl;
			m_outFile << "Call count = " << static_cast<unsigned int>(m_checker.get_count()) << std::endl;
			m_outFile << "Average time = " << m_checker.get_average_time() << std::endl;
			m_outFile << "===============================================" << std::endl;
		}
};
}

/// ������ ������� ����� ��� ���������� ����������
#define DECLARE_PERFORMANCE_FILE_STREAM(fileName, varName)    \
	static std::ofstream varName(#fileName)

/// ������ ������� ������������ ���������� checkera
#define DECLARE_PERFORMANCE_CHECKER(index, outFile)    \
	static performance::stream_guard performance_checker_##index(index, outFile)

/// ������ ������ ������
#define START_PERFORMANCE_CHECK(index)    \
	performance_checker_##index.start()

/// ������ ����� ������
#define FINISH_PERFORMANCE_CHECK(index)    \
	performance_checker_##index.finish()
