#pragma once


#include <iostream>
#include <fstream>
#include <sstream>


// function to convert a string to a specified type T.
template<typename T>
T convertFromString(const std::string &str) {
    if constexpr (std::is_same_v<T, std::string>) {
        // if target type is std::string -> return input string.
        return str;
    } else {
        T value;
        std::stringstream ss(str);
        ss >> value;  // Try to convert to type T

        if (ss.fail() || !ss.eof()) {
            throw std::invalid_argument("Can't convert '" + str + "' into " + typeid(T).name());
        }

        return value;
    }
}


// exception class for handling CSV parsing errors.
class CSVParseException final : public std::runtime_error {
public:
    CSVParseException(const std::string &msg, const int line, const int column)
        : runtime_error(msg), line(line), column(column) {
    }

    int getLine() const { return line; }
    int getColumn() const { return column; }

private:
    int line;
    int column;
};


// class for representing a CSV row with a tuple of various types
template<typename... Args>
class CSVRow {
public:
    explicit CSVRow(const std::string &line = "", const char delimiter = ',', const char quote = '"', const size_t lineNumber=0)
        : LineNumber(lineNumber), delimiter(delimiter), quote(quote) {
        if (!line.empty()) {
            parse(line);
        }
    }

    // get values of row as tuple
    std::tuple<Args...> getValues() const {
        return convertToTuple<0, Args...>();
    }

private:
    size_t LineNumber = 0;
    std::vector<std::string> values;  // hold the string values in the row
    char delimiter;
    char quote;

    // parse the CSV line into separated cells (values)
    void parse(const std::string &line) {
        std::stringstream ss(line);
        std::string cell;
        bool inQuotes = false;

        while (getline(ss, cell, delimiter)) {
            if (!inQuotes) {
                if (cell.front() == quote && cell.back() == quote) {
                    cell = cell.substr(1, cell.size() - 2);  // remove surrounding quotes

                } else if (cell.front() == quote) {
                    inQuotes = true;
                    cell = cell.substr(1);  // remove opening quote
                }

            } else {
                if (cell.back() == quote) {
                    inQuotes = false;
                    cell = cell.substr(0, cell.size() - 1);  // remove closing quote.
                }
            }
            values.push_back(cell);
        }
    }

    // recursively converts the row values into a tuple.
    template<size_t Index, typename T, typename... Rest>
    std::tuple<T, Rest...> convertToTuple() const {
        std::tuple<T, Rest...> t;  // tuple to store the current value and remaining values

        try {
            std::get<0>(t) = convertFromString<T>(values[Index]);  // convert string to the correct type.
        } catch (...) {
            throw CSVParseException("Error parsing value", LineNumber, Index);
        }

        if constexpr (sizeof...(Rest) > 0) {
            // recursively convert the remaining values into the rest of the tuple
            auto rest = convertToTuple<Index + 1, Rest...>();
            return tuple_cat(std::tuple<T>(std::get<0>(t)), rest);  // combine the current value with the rest
        }

        return t;  // fully constructed tuple
    }

    // case when there is only one value in the row
    template<typename T>
    std::tuple<T> convertToTuple() const {
        std::tuple<T> t;
        try {
            std::get<0>(t) = convertFromString<T>(values[0]);  // convert the first value to the required type
        } catch (...) {
            throw CSVParseException("Error parsing value", 0, 0);
        }

        return t;
    }
};


// reading CSV rows one by one.
template<typename... Args>
class CSVParserIterator {
public:
    using value_type = std::tuple<Args...>;  // tuple of values for each row

    explicit CSVParserIterator(std::ifstream *file, const bool end = false, char delimiter = ',', char quote = '"')
        : file(file), currentRow("", delimiter, quote, 0), end(end), lineNumber(0) {
        if (!end) {
            advance();  // move iterator to the first row
        }
    }

    // operator returns the current row as a tuple.
    value_type operator*() {
        return currentRow.getValues();
    }

    // operator moves to the next row in the file.
    CSVParserIterator &operator++() {
        advance();  // move to the next row.
        return *this;
    }

    bool operator!=(const CSVParserIterator &other) const {
        return end != other.end;
    }

private:
    std::ifstream *file;
    CSVRow<Args...> currentRow;
    bool end;                     // indicating the iterator has reached the end of the file
    int lineNumber;

    // advance the iterator to the next row.
    void advance() {
        if (std::string line; getline(*file, line)) {
            lineNumber++;
            currentRow = CSVRow<Args...>(line, ',', '"', lineNumber);  // parse next row

        } else {
            end = true;  // if no more lines mark the iterator as ended
        }
    }
};


// main CSV parser class for reading and parsing CSV files into rows of tuples
template<typename... Args>
class CSVParser {
public:
    // optionally skipping a number of lines at the start.
    explicit CSVParser(std::ifstream &file, const size_t skipLines = 0, const char delimiter = ',', const char quote = '"')
        : file(file), delimiter(delimiter), quote(quote) {

        for (size_t i = 0; i < skipLines; ++i) {  // Skip the specified number of lines.
            std::string line;
            getline(file, line);
        }
    }

    // beginning of the CSV data.
    CSVParserIterator<Args...> begin() {
        return CSVParserIterator<Args...>(&file, false, delimiter, quote);
    }

    // end of the CSV data (past the last element).
    CSVParserIterator<Args...> end() {
        return CSVParserIterator<Args...>(&file, true, delimiter, quote);
    }

private:
    std::ifstream &file;
    char delimiter;
    char quote;
};


// recursively print the values of a tuple separated by commas
template <std::size_t idx, typename... Args>
void print_tuple(std::ostream& stream, const std::tuple<Args...>& t) {
    if constexpr (idx < sizeof...(Args)) {
        if (idx != 0) {
            stream << ", ";
        }
        stream << std::get<idx>(t);

        print_tuple<idx + 1>(stream, t);  // Recursively print elements
    }
}


// print the contents of a tuple.
template <typename... Args>
auto operator<<(std::ostream& stream, const std::tuple<Args...>& t) -> std::ostream& {
    if constexpr (sizeof...(Args) == 0) {
        stream << "{}";  // if the tuple is empty
    } else {
        stream << "{";
        print_tuple<0>(stream, t);
        stream << "}";
    }

    return stream;
}
