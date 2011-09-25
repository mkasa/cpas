#ifndef _TSV_HEADER
#define _TSV_HEADER

#include <string_piece.h>
#include <cstdio>
#include <cassert>

class FastTSVParse {
	static const int BUFFER_SIZE = 16 * 1024 * 1024;
	static const int FP_BUFFER_SIZE = 256 * 1024;
	static const int MAX_COLUMNS = 2048;
	char* buffer;
	char* fpbuffer;
	FILE* fp;
	int line_counter;
	std::vector<base::StringPiece> columns;

	void init() {
		buffer = new char[BUFFER_SIZE];
		fpbuffer = new char[FP_BUFFER_SIZE];
		fp = NULL;
		line_counter = 0;
		columns.reserve(20);
	}
public:
	bool open(const std::string& fileName) {
		fp = fopen(fileName.c_str(), "rb");
		if(fp != NULL) {
			setbuffer(fp, fpbuffer, FP_BUFFER_SIZE);
		}
		line_counter = 0;
		return fp != NULL;
	}
	bool readNextLine() {
		char* p = fgets(buffer, BUFFER_SIZE, fp);
		if(p == NULL) return false;
		line_counter++;
		columns.resize(0);
		while(true) {
			char* ep = p;
			while(*ep != '\0' && *ep != '\t' && *ep != '\n' && *ep != '\n') ep++;
			if(ep == p)	break;
			columns.push_back(base::StringPiece(p, ep - p));
			if(*ep == '\t') ep++;
			p = ep;
		}
		return true;
	}
	inline int getLineNumber() const { return line_counter; }
	inline std::size_t size() const { return columns.size(); }
	operator bool () const { return fp != NULL; }
	const char * c_str() const { return buffer; }
	const base::StringPiece& operator [] (size_t index) const {
		assert(index < size());
		return columns[index];
	}
	std::string getString (size_t index) const {
		assert(index < size());
		return columns[index].as_string();
	}
	bool isEmpty (size_t index) const {
		assert(index < size());
		return columns[index].empty();
	}		
	int getInteger (size_t index) const {
		assert(index < size());
		const base::StringPiece& s = columns[index];
		if(s.empty()) return 0;
		bool isMinus = false;
		int p = 0;
		if(s[0] == '-') {
			isMinus = true;
			p++;
		}
		int rval = 0;
		if(p < s.size()) {
			if(!('0' <= s[p] && s[p] <= '9'))	return 0;
			rval = s[p++] - '0';
		}
		while(p < s.size()) {
			if(!('0' <= s[p] && s[p] <= '9'))	return isMinus ? -rval : rval;
			rval = rval * 10 + s[p++] - '0';
		}
		return isMinus ? -rval : rval;
	}
	void close() {
		if(fp) {
			fclose(fp);
			fp = NULL;
		}
	}
	FastTSVParse() {
		init();
	}
	FastTSVParse(const std::string& fileName) {
		init();
		open(fileName);
	}
	~FastTSVParse() {
		close();
		delete[] buffer;
		delete[] fpbuffer;
	}
};


#endif // #ifndef _TSV_HEADER

