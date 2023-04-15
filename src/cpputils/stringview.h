#ifndef STRINGVIEW_H
#define STRINGVIEW_H

#include <string_view>

std::string_view strViewTrim(std::string_view string);      // Remove leading and trailing spaces and newlines


class next_line_view_cursor;
std::string_view next_line_view(next_line_view_cursor *cursor);

class next_line_view_cursor
{
    friend std::string_view next_line_view(next_line_view_cursor *cursor);
    const char * m_current_position;
    const char * m_last_position;
public:
    next_line_view_cursor(const char *current_position, size_t size_bytes) :
        m_current_position(current_position), m_last_position(current_position + size_bytes)
    {}
    //next_line_view_cursor& operator=(next_line_view_cursor const&) = default;

    inline bool canReadMore() noexcept {
        return (m_current_position < m_last_position);
    }
};


#endif // STRINGVIEW_H
