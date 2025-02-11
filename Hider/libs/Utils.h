#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;

#define GETINDEXVEC(vec, val) (std::find(vec.begin(), vec.end(), val) - vec.begin())
#define FINDVECTOR(vec, val) (vec.size() && std::find(vec.begin(), vec.end(), val) != vec.end())
#define ERASEVECTOR(vec, index) vec.erase(vec.begin() + index)
#define ERASEVECTORVAL(vec, val) if(FINDVECTOR(vec, val)) vec.erase(vec.begin() + GETINDEXVEC(vec, val)); else {}

inline std::wstring to_wstring(std::string str) {
	return std::wstring(str.begin(), str.end());
}

inline std::string to_string(std::wstring str) {
	return std::string(str.begin(), str.end());
}

static char* ReadAllBytes(wstring filename, size_t* read)
{
	ifstream ifs(filename, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();
	int length = pos;
	char* pChars = new char[length];
	ifs.seekg(0, ios::beg);
	ifs.read(pChars, length);
	ifs.close();
	*read = length;
	return pChars;
}

static char* ReadAllBytes(string filename, size_t* read)
{
	return ReadAllBytes(to_wstring(filename), read);
}

template< typename T >
std::string to_hex(T i, int count = 0)
{
	std::stringstream stream;
	uint64_t bytes = 0xFFFFFFFFFFFFFFFF;
	if (count == 0)
		count = sizeof(T);
	count *= 8;
	bytes = bytes >> sizeof(bytes) * 8 - count;
	stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << (bytes & i);
	return stream.str();
}

static string hashString(string str, int size = 4) {
	size /= 2;
	string temp;
	if (str.size() < size) {
		temp = str;
		for (int i = str.size(); i < size; i++) {
			temp += "\0";
		}
	}
	else {
		temp = string(str, 0, size);
	}
	string result = "";

	for (int i = 0; i < str.size() || i < size; i++) {
		temp[i % temp.size()] += str[i % str.size()];
	}

	for (int i = 0; i < temp.size(); i++) {
		result += to_hex((char)temp[i]);
	}


	return result;
}

static std::string ReplaceAll(std::string str, const std::string& from = "", const std::string& to = "") {
	if (from == "" && to == "") {
		string resultStr = "";
		for (int i = 0; i < str.size(); i++) {
			if (std::isdigit((unsigned char)str[i]) || std::ispunct((unsigned char)str[i]) || std::isalpha((unsigned char)str[i]))
			{
				resultStr += str[i];
			}
			else {
				resultStr += hashString(string("_") + str[i] + "_");
			}
		}
		return resultStr;
	}

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

static string SizeToString(size_t size) {
	string res;
	if (size > 1024 * 1024) {
		res = to_string((float)size / 1024 / 1024);
		if (res.find('.') != string::npos)
			res = res.substr(0, res.find('.') + 3);
		res += " MB";
	}
	else if (size > 1024) {
		res = to_string((float)size / 1024);
		if (res.find('.') != string::npos)
			res = res.substr(0, res.find('.') + 3);
		res += " KB";
	}
	else {
		res = to_string(size) + " B";
	}
	return res;
}

static string lowercase(const string& s)
{
	std::string str(s);
	std::transform(str.begin(), str.end(), str.begin(),
		[](char c) { return std::tolower(c); });
	return str;
}

static wstring wlowercase(const wstring& s)
{
	std::wstring str(s);
	std::transform(str.begin(), str.end(), str.begin(),
		[](wchar_t c) { return std::tolower(c); });
	return str;
}

static void WriteToFile(std::string FileName, std::string text) {
	std::ofstream myfile;
	myfile.open(FileName, std::ios_base::app);
	myfile << text << "\n";
	myfile.close();
}

static void WriteToFile(std::wstring FileName, std::string text) {
	std::ofstream myfile;
	myfile.open(FileName, std::ios_base::app);
	myfile << text << "\n";
	myfile.close();
}

static void WriteToFile(std::wstring FileName, char* file, size_t size) {
	std::ofstream myfile;
	myfile.open(FileName, std::ios_base::out | std::ios::binary);
	myfile.write(file, size);
	myfile.close();
}

static void WriteToFile(std::string FileName, char* file, size_t size) {
	std::ofstream myfile;
	myfile.open(FileName, std::ios_base::out | std::ios::binary);
	myfile.write(file, size);
	myfile.close();
}

static int align(int input, int align) {
	return align * (int)ceil((float)input / (float)align);
}

static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

template<typename T2, typename T1>
inline T2 lexical_cast(const T1& in) {
	T2 out;
	std::stringstream ss;
	ss << in;
	ss >> out;
	return out;
}

static bool isASCII(const std::string& s)
{
	return !std::any_of(s.begin(), s.end(), [](char c) {
		return static_cast<unsigned char>(c) > 127;
		});
}