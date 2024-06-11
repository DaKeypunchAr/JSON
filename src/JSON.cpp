#include "JSON.hpp"

namespace JSON
{
	/* JSON Initialization helping methods */
	void JSON::initialize(const std::string& fileLoc)
	{
		/* Reading the File */
		std::ifstream fReader(fileLoc);
		std::string jsonFile;
		for (std::string line; std::getline(fReader, line); jsonFile += '\n') jsonFile += line;

		// Use std::remove to move the characters to be removed to the end of the string - Removing the new lines actually.
		jsonFile.erase(std::remove(jsonFile.begin(), jsonFile.end(), '\n'), jsonFile.end());

		if (jsonFile.at(0) != '{')
		{
			std::cout << "JSON not supported!\n";
			return;
		}

		m_StartBlock = evaluateBlock(jsonFile);
	}
	JSONBlock JSON::evaluateBlock(const std::string& jsonBlock)
	{
		int decrement = 2;
		for (unsigned int i = jsonBlock.length() - 1; i > 0; i--)
		{
			if (jsonBlock[i] == ' ') decrement++;
			else break;
		}

		std::string keyValuePairs = jsonBlock.substr(1, jsonBlock.length() - decrement);
		std::vector<std::string> keyValuePairsV;
		std::istringstream stream(keyValuePairs);
		for (std::string pair; std::getline(stream, pair, ','); ) keyValuePairsV.push_back(pair);

		JSONBlock block;

		for (unsigned int i = 0; i < keyValuePairsV.size(); i++)
		{
			std::string pair = keyValuePairsV[i];
			/* Taking care of special case of lists */
			int list;
			int iterations = 1;
			do
			{
				list = std::count(pair.begin(), pair.end(), '[') - std::count(pair.begin(), pair.end(), ']');
				if (list != 0)
				{
					pair += ',';
					pair += keyValuePairsV[i + iterations];
					iterations++;
				}
				if (i + iterations > keyValuePairsV.size()) break;
			} while (list != 0);
			do
			{
				list = std::count(pair.begin(), pair.end(), '{') - std::count(pair.begin(), pair.end(), '}');
				if (list != 0)
				{
					pair += ',';
					pair += keyValuePairsV[i + iterations];
					iterations++;
				}
				if (i + iterations > keyValuePairsV.size()) break;
			} while (list != 0);

			block.m_Pairs.push_back(evaluateKeyValPair(pair));

			i += iterations - 1;
		}
		return block;
	}
	JSONValue JSON::evaluateValue(const std::string& value)
	{
		JSONValue val;
		std::string data;
		int i = 0;

		bool dataTypeFound = false;
		do
		{
			if (value[i] == ' ' || value[i] == ':')
			{
				i++;
				continue;
			}
			if (value[i] == '"')
			{
				val.type = JSONDataType::STRING;
				dataTypeFound = true;
			}
			else if (value[i] == '{')
			{
				val.type = JSONDataType::JSON_BLOCK;
				dataTypeFound = true;
			}
			else if (value[i] == '[')
			{
				val.type = JSONDataType::LIST;
				dataTypeFound = true;
			}
			else if (value[i] == 'n')
			{
				val.type = JSONDataType::_NULL;
				dataTypeFound = true;
			}
			else
			{
				val.type = JSONDataType::NUMBER;
				dataTypeFound = true;
			}
			i++;
		} while (!dataTypeFound);
		data = value.substr(i - 1);

		unsigned int start, end;
		switch (val.type)
		{
		case JSONDataType::NUMBER:
			start = data.find_first_not_of(" \t\r\n");
			end = data.find_last_not_of(" \t\r\n");
			data = data.substr(start, end - start + 1);
			try {
				// Attempt to convert to long long
				long long num = std::strtoll(data.c_str(), nullptr, 10);
				val.num = num;
			}
			catch (...) {
				double num = std::strtod(data.c_str(), nullptr);
				val.num = num;
			}
			break;
		case JSONDataType::STRING:
			start = data.find_first_of('"') + 1;
			end = data.find_last_of('"') -1;
			val.str = data.substr(start, end - start + 1);
			break;
		case JSONDataType::LIST:
			start = data.find_first_not_of(" \t\r\n");
			end = data.find_last_not_of(" \t\r\n");
			data = data.substr(start, end - start + 1);
			val.list = evaluateList(data);
			break;
		case JSONDataType::JSON_BLOCK:
			val.child = evaluateBlock(data);
		}
		return val;
	}
	JSONKeyValuePair JSON::evaluateKeyValPair(const std::string& pair)
	{
		bool keyFound = false;
		bool keyStartFound = false;
		std::string key;
		int i{};
		do
		{
			if (pair[i] == ' ')
			{
				i++;
				continue;
			}
			if (pair[i] == '"')
			{
				if (!keyStartFound)
				{
					keyStartFound = true;
					i++;
					continue;
				}
				else keyFound = true;
			}
			if (!keyFound)
			{
				key += pair[i];
			}
			i++;
		} while (!keyFound);


		JSONKeyValuePair _pair;
		_pair.key = key;
		JSONValue val = evaluateValue(pair.substr(i));
		_pair.value = val;

		return _pair;
	}
	std::vector<JSONValue> JSON::evaluateList(const std::string& list)
	{
		std::vector<std::string> itemList;
		std::istringstream stream(list.substr(1, list.length() - 2));
		for (std::string _list; std::getline(stream, _list, ','); ) itemList.push_back(_list);

		std::vector<JSONValue> valList;

		for (unsigned int i = 0; i < itemList.size(); i++)
		{
			std::string item = itemList[i];

			JSONValue val;

			/* Detecting the type! */
			for (unsigned int j = 0; j < item.length(); j++)
			{
				if (item[j] == ' ')
				{
					j++;
					continue;
				}
				else if (item[j] == '"')
				{
					val.type = JSONDataType::STRING;
				}
				else if (item[j] == '{')
				{
					val.type = JSONDataType::JSON_BLOCK;
				}
				else if (item[j] == '[')
				{
					val.type = JSONDataType::LIST;
				}
				else if (item[j] == 'n')
				{
					break;
				}
				else
				{
					val.type = JSONDataType::NUMBER;
				}
				if (val.type != JSONDataType::_NULL) break;
				j++;
			}

			int diff;
			int iterations = 1;
			switch (val.type)
			{
			case JSONDataType::LIST:
				do
				{
					diff = std::count(item.begin(), item.end(), '[') - std::count(item.begin(), item.end(), ']');
					if (diff != 0)
					{
						item += ',';
						item += itemList[i + iterations];
					}
					iterations++;
				} while (diff != 0);
				i += iterations - 2;
				val = evaluateValue(item);
				break;
			case JSONDataType::JSON_BLOCK:
				do
				{
					diff = std::count(item.begin(), item.end(), '{') - std::count(item.begin(), item.end(), '}');
					if (diff != 0)
					{
						item += ',';
						item += itemList[i + iterations];
					}
					iterations++;
				} while (diff != 0);
				i += iterations - 2;
				val = evaluateValue(item);
				break;
			case JSONDataType::NUMBER:
			case JSONDataType::STRING:
				val = evaluateValue(item);
			}

			valList.push_back(val);
		}
		return valList;
	}

	/* JSON Printing Methods */
	std::string JSONBlock::getPrintableVersion(unsigned int depth, bool startWithDepth, bool addComma) const
	{
		if (m_HasPrintableVersion) return m_PrintableVersion;
		std::string depthTabbed;
		for (unsigned int i = 0; i < depth; i++) depthTabbed += "   ";

		if (startWithDepth) m_PrintableVersion += depthTabbed + "{\n";
		else m_PrintableVersion += "{\n";

		for (unsigned int i = 0; i < m_Pairs.size(); i++)
		{
			m_PrintableVersion += m_Pairs[i].getPrintableVersion(depth + 1, i + 1 != m_Pairs.size());
		}

		m_PrintableVersion += depthTabbed + ((addComma) ? "},\n" : "}\n");
		m_HasPrintableVersion = true;

		return m_PrintableVersion;
	}
	std::string JSONKeyValuePair::getPrintableVersion(unsigned int depth, bool addComma) const
	{
		if (m_HasPrintableVersion) return m_PrintableVersion;
		std::string depthTabbed;
		for (unsigned int i = 0; i < depth; i++) depthTabbed += "   ";

		m_PrintableVersion += depthTabbed + key + "\" : ";
		m_PrintableVersion += value.getPrintableVersion(depth, false, addComma);

		m_HasPrintableVersion = true;
		return m_PrintableVersion;
	}
	std::string JSONValue::getPrintableVersion(unsigned int depth, bool tabbed, bool addComma) const
	{
		if (m_HasPrintableVersion) return m_PrintableVersion;
		std::string depthTabbed;
		for (unsigned int i = 0; i < depth; i++) depthTabbed += "   ";
		switch (type)
		{
		case JSONDataType::_NULL:
			if (tabbed) m_PrintableVersion += depthTabbed;
			m_PrintableVersion += ((addComma) ? "null,\n" : "null\n");
			break;
		case JSONDataType::NUMBER:
			if (tabbed) m_PrintableVersion += depthTabbed;
			m_PrintableVersion += num.getPrintableVersion(addComma);
			break;
		case JSONDataType::STRING:
			if (tabbed) m_PrintableVersion += depthTabbed;
			m_PrintableVersion += str + ((addComma) ? ",\n" : "\n");
			break;
		case JSONDataType::JSON_BLOCK:
			m_PrintableVersion += child.getPrintableVersion(depth, tabbed, addComma);
			break;
		case JSONDataType::LIST:
			if (tabbed) m_PrintableVersion += depthTabbed;
			m_PrintableVersion += "[\n";
			for (unsigned int i = 0; i < list.size(); i++)
			{
				m_PrintableVersion += list[i].getPrintableVersion(depth + 1 + ((tabbed) ? 1 : 0), true, i + 1 != list.size());
			}
			m_PrintableVersion += depthTabbed + ((addComma) ? "],\n" : "]\n");
		}
		m_HasPrintableVersion = true;
		return m_PrintableVersion;
	}

	void JSON::print() const
	{
		if (m_HasPrintableVersion)
		{
			std::cout << m_PrintableVersion;
			return;
		}
		m_PrintableVersion = m_StartBlock.getPrintableVersion(0, false, false);
		m_HasPrintableVersion = true;
		std::cout << m_PrintableVersion;
	}
	void JSONBlock::print() const
	{
		if (m_HasPrintableVersion)
		{
			std::cout << m_PrintableVersion;
			return;
		}
		m_PrintableVersion = getPrintableVersion(0, false, false);
		m_HasPrintableVersion = true;
		std::cout << m_PrintableVersion;
	}
	void JSONKeyValuePair::print() const
	{
		if (m_HasPrintableVersion)
		{
			std::cout << m_PrintableVersion;
			return;
		}
		m_PrintableVersion = getPrintableVersion(0, false);
		m_HasPrintableVersion = true;
		std::cout << m_PrintableVersion;
	}
	void JSONValue::print() const
	{
		if (m_HasPrintableVersion)
		{
			std::cout << m_PrintableVersion;
			return;
		}
		m_PrintableVersion = getPrintableVersion(0, false, false);
		m_HasPrintableVersion = true;
		std::cout << m_PrintableVersion;
	}

	/* JSON using methods */
	const JSONValue& JSONBlock::operator[](const char* key) const
	{
		for (const JSONKeyValuePair& p : m_Pairs)
		{
			if (p.key == key) return p.value;
		}
		throw std::logic_error("Key not matched!");
	}
	JSONValue& JSONBlock::operator[](const char* key)
	{
		for (JSONKeyValuePair& p : m_Pairs)
		{
			if (p.key == key) return p.value;
		}
		throw std::logic_error("Key not matched!");
	}

	const JSONValue& JSON::operator[](const char* key) const
	{
		return m_StartBlock[key];
	}
	JSONValue& JSON::operator[](const char* key)
	{
		return m_StartBlock[key];
	}

	const JSONValue& JSONValue::operator[](const char* key) const
	{
		if (type == JSONDataType::JSON_BLOCK)
			return child[key];
		throw std::logic_error("JSONValue is not a block!");
	}
	JSONValue& JSONValue::operator[](const char* key)
	{
		if (type == JSONDataType::JSON_BLOCK)
			return child[key];
		throw std::logic_error("JSONValue is not a block!");
	}
	const JSONValue& JSONValue::operator[](int i) const
	{
		if (type == JSONDataType::LIST)
		{
			if (i > list.size() || i < 0) throw std::out_of_range("Index out of bounds!\n");
			return list[i];
		}
		throw std::logic_error("JSONValue is not an list!");
	}
	JSONValue& JSONValue::operator[](int i)
	{
		if (type == JSONDataType::LIST)
		{
			if (i > list.size() || i < 0) throw std::out_of_range("Index out of bounds!\n");
			return list[i];
		}
		throw std::logic_error("JSONValue is not an list!");
	}

	JSONValue::operator double() const
	{
		if (type != JSONDataType::NUMBER) throw std::bad_cast();
		if (num.holdsDouble()) return num.getDouble();
		return (double)num.getLongLong();
	}
	JSONValue::operator long long() const
	{
		if (type != JSONDataType::NUMBER) throw std::bad_cast();
		if (num.holdsLongLong()) return num.getLongLong();
		return (long long)num.getDouble();
	}
	JSONValue::operator int() const
	{
		int i = (long long)*this;
		return i;
	}
	JSONValue::operator unsigned int() const
	{
		unsigned int i = (long long)*this;
		return i;
	}
	JSONValue::operator short() const
	{
		short s = (long long)*this;
		return s;
	}
	JSONValue::operator unsigned short() const
	{
		unsigned short s = (long long)*this;
		return s;
	}
	JSONValue::operator char() const
	{
		char c = (long long)*this;
		return c;
	}
	JSONValue::operator unsigned char() const
	{
		unsigned char c = (long long)*this;
		return c;
	}
	JSONValue::operator std::string() const
	{
		if (type == JSONDataType::STRING) return str;
		throw std::bad_cast();
	}
	JSONValue::operator const char*() const
	{
		if (type != JSONDataType::STRING) throw std::bad_cast();
		return str.c_str();
	}
	JSONValue::operator const std::vector<JSONValue>&() const
	{
		if (type != JSONDataType::LIST) throw std::bad_cast();
		return list;
	}
	JSONValue::operator std::vector<JSONValue>& ()
	{
		if (type != JSONDataType::LIST) throw std::bad_cast();
		return list;
	}

	unsigned int JSONValue::size() const
	{
		if (type != JSONDataType::LIST) throw std::runtime_error("The value is not an list!\n");
		return list.size();
	}
}