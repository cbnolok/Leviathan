#include "stringview.h"
#include <algorithm>

std::string_view strViewTrim(std::string_view string)
{
    static constexpr char delims[] = " \t\n";

    size_t start = string.find_first_not_of(delims);        // Finds the first character equal to none of characters in delims
    if (start == std::string_view::npos)
        start = 0;

    size_t nchars = string.find_last_not_of(delims);        // Finds the last character equal to none of characters in delims
    if (nchars != std::string_view::npos)
        nchars = nchars - start + 1;

    return string.substr(start, nchars);
}

// like std::getline
std::string_view next_line_view(next_line_view_cursor *cursor)
{
  const char* end = std::find(cursor->m_current_position, cursor->m_last_position, '\n');
  std::string_view result(cursor->m_current_position, end - cursor->m_current_position);

  if (end == cursor->m_last_position)
    cursor->m_current_position = end;
  else
    cursor->m_current_position = end + 1;

  return result;
}
