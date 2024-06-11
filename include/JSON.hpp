#ifndef _JSON_HPP
#define _JSON_HPP

#include <variant>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

namespace JSON
{
	/* Data types available in JSON */
	enum class JSONDataType
	{
		NUMBER,
		STRING,
		LIST,
		JSON_BLOCK,
		_NULL
	};

	/* A Number Class Just for extracting JSON easily */
	struct Number
	{
		Number(char n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(unsigned char n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(short n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(unsigned short n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(int n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(unsigned int n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(long n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(unsigned long n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(long long n)
			: m_Number(n), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(unsigned long long n)
			: m_Number(static_cast<long long>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(float n)
			: m_Number(static_cast<double>(n)), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number(double n)
			: m_Number(n), m_HasPrintableVersion(false), m_PrintableVersion() {}
		Number()
			: m_Number(-9223372036854775807), m_HasPrintableVersion(false), m_PrintableVersion() {}

		long long getLongLong() const
		{
			return std::get<long long>(m_Number);
		}
		double getDouble() const
		{
			return std::get<double>(m_Number);
		}
		std::variant<long long, double> get() const { return m_Number; }

		bool holdsLongLong() const
		{
			return std::holds_alternative<long long>(m_Number);
		}
		bool holdsDouble() const
		{
			return std::holds_alternative<double>(m_Number);
		}

		void print() const
		{
			if (m_HasPrintableVersion)
			{
				std::cout << m_PrintableVersion;
				return;
			}
			m_PrintableVersion = getPrintableVersion(false);
			m_HasPrintableVersion = true;
			std::cout << m_PrintableVersion;
		}
		std::string getPrintableVersion(bool addComma) const
		{
			if (m_HasPrintableVersion) return m_PrintableVersion;

			m_PrintableVersion += std::visit([](auto&& arg) -> std::string { return std::to_string(arg); }, m_Number);
			m_PrintableVersion += ((addComma) ? ",\n" : "\n");
			m_HasPrintableVersion = true;
			return m_PrintableVersion;
		}

		std::variant<long long, double> m_Number;
		mutable bool m_HasPrintableVersion = false;
		mutable std::string m_PrintableVersion;
	};

	class JSON;
	class JSONValue;
	class JSONKeyValuePair;

	class JSONBlock
	{
	public:
		void print() const;
		std::string getPrintableVersion(unsigned int depth, bool startWithDepth = true, bool addComma = true) const;

		const JSONValue& operator[](const char* key) const;
		JSONValue& operator[](const char* key);

		const std::string& getKey(unsigned int index) const;
		std::string& getKey(unsigned int index);

		unsigned int totalKeys() const { return m_Pairs.size(); }

		bool exists(const std::string& key) const;

	private:
		std::vector<JSONKeyValuePair> m_Pairs;
		mutable bool m_HasPrintableVersion = false;
		mutable std::string m_PrintableVersion;

		friend JSON;
	};

	struct JSONValue
	{
		JSONDataType type = JSONDataType::_NULL;
		Number num;
		std::string str;
		std::vector<JSONValue> list;
		JSONBlock child;

		const JSONValue& operator[](const char* key) const;
		JSONValue& operator[](const char* key);
		const JSONValue& operator[](int) const;
		JSONValue& operator[](int);

		const std::string& getKey(unsigned int index) const { return child.getKey(index); }
		std::string& getKey(unsigned int index) { return child.getKey(index); }

		unsigned int totalKeys() const { return child.totalKeys(); }

		operator double() const;
		operator long long() const;
		operator int() const;
		operator unsigned int() const;
		operator short() const;
		operator unsigned short() const;
		operator char() const;
		operator unsigned char() const;
		operator std::string() const;
		operator const char*() const;
		operator const std::vector<JSONValue>&() const;
		operator std::vector<JSONValue>&();

		bool exists(const std::string& key) const;

		unsigned int size() const;

		void print() const;
		std::string getPrintableVersion(unsigned int depth, bool tabbed = false, bool addComma = true) const;
		mutable bool m_HasPrintableVersion = false;
		mutable std::string m_PrintableVersion;
	};

	struct JSONKeyValuePair
	{
		std::string key;
		JSONValue value;
		void print() const;
		std::string getPrintableVersion(unsigned int depth, bool addComma = true) const;
		mutable bool m_HasPrintableVersion = false;
		mutable std::string m_PrintableVersion;
	};

	class JSON
	{
	public:
		JSON(const std::string& fileLoc)
		{
			initialize(fileLoc);
		}

		void initialize(const std::string& fileLoc);
		void print() const;

		const JSONValue& operator[](const char* key) const;
		JSONValue& operator[](const char* key);

		const std::string& getKey(unsigned int index) const { return m_StartBlock.getKey(index); }
		std::string& getKey(unsigned int index) { return m_StartBlock.getKey(index); }

		unsigned int totalKeys() const { return m_StartBlock.totalKeys(); }

		JSONBlock getRootBlock() const { return m_StartBlock; }

		bool exists(const std::string& key) const;

	private:
		JSONBlock evaluateBlock(const std::string& jsonBlock);
		JSONValue evaluateValue(const std::string& value);
		std::vector<JSONValue> evaluateList(const std::string& list);
		JSONKeyValuePair evaluateKeyValPair(const std::string& pair);

	private:
		JSONBlock m_StartBlock;
		mutable bool m_HasPrintableVersion = false;
		mutable std::string m_PrintableVersion;
	};
}

#endif