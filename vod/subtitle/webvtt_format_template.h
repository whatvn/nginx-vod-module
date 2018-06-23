static int64_t
METHOD(webvtt_read_timestamp)(CHAR_TYPE* cur_pos, CHAR_TYPE** end_pos)
{
	int64_t hours;
	int64_t minutes;
	int64_t seconds;
	int64_t millis;
	int8_t sign;

	// minus
	if (*cur_pos == '-')
	{
		cur_pos++;
		sign = 0;		// clamp the timestamp to zero (negative is interpreted as error)
	}
	else
	{
		sign = 1;
	}

	// hour digits
	if (!isdigit(*cur_pos))
	{
		return -1;
	}

	hours = 0;
	for (; isdigit(*cur_pos); cur_pos++)
	{
		hours = hours * 10 + (*cur_pos - '0');
	}

	// colon
	if (*cur_pos != ':')
	{
		return -1;
	}
	cur_pos++;

	// 2 minute digits
	if (!isdigit(cur_pos[0]) || !isdigit(cur_pos[1]))
	{
		return -1;
	}
	minutes = (cur_pos[0] - '0') * 10 + (cur_pos[1] - '0');
	cur_pos += 2;

	// colon
	if (*cur_pos == ':')
	{
		cur_pos++;

		// 2 second digits
		if (!isdigit(cur_pos[0]) || !isdigit(cur_pos[1]))
		{
			return -1;
		}
		seconds = (cur_pos[0] - '0') * 10 + (cur_pos[1] - '0');
		cur_pos += 2;
	}
	else
	{
		// no hours
		seconds = minutes;
		minutes = hours;
		hours = 0;
	}

	// dot
	if (*cur_pos != '.' && *cur_pos != ',')
	{
		if (end_pos != NULL)
		{
			*end_pos = cur_pos;
		}

		return sign * 1000 * (seconds + 60 * (minutes + 60 * hours));
	}
	cur_pos++;

	// 1-3 digit millis
	if (!isdigit(cur_pos[0]))
	{
		return -1;
	}

	millis = (*cur_pos++ - '0') * 100;

	if (isdigit(*cur_pos))
	{
		millis += (*cur_pos++ - '0') * 10;

		if (isdigit(*cur_pos))
		{
			millis += (*cur_pos++ - '0');

			while (isdigit(*cur_pos))
			{
				cur_pos++;
			}
		}
	}

	if (end_pos != NULL)
	{
		*end_pos = cur_pos;
	}

	return sign * (millis + 1000 * (seconds + 60 * (minutes + 60 * hours)));
}

static bool_t
METHOD(webvtt_identify_srt)(CHAR_TYPE* p)
{
	for (; isspace(*p); p++);

	// n digits
	if (!isdigit(*p))
	{
		return FALSE;
	}

	for (; isdigit(*p); p++);

	for (; *p == ' ' || *p == '\t'; p++);

	// new line
	switch (*p)
	{
	case '\r':
		p++;
		if (*p == '\n')
		{
			p++;
		}
		break;

	case '\n':
		p++;
		break;

	default:
		return FALSE;
	}

	// timestamp
	if (METHOD(webvtt_read_timestamp)(p, &p) < 0)
	{
		return FALSE;
	}

	for (; *p == ' ' || *p == '\t'; p++);

	// cue marker
	return p[0] == '-' && p[1] == '-' && p[2] == '>';
}
