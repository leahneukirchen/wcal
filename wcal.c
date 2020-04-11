/*
 * wcal - ISO weekly calendar
 *
 * To the extent possible under law, Leah Neukirchen <leah@vuxu.org>
 * has waived all copyright and related or neighboring rights to this work.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

/*
 * -1 show current month (default)
 * -3 show three months
 * -y show whole year
 * -c show current week only
 * -i print infinite calendar
 * -d YYYY-MM-DD different value for today
 */

#define _XOPEN_SOURCE 700
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int flag1, flag3, flagc, flagi, flagy;

void
parse_isodate(char *optarg, struct tm *tm)
{
	tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
	tm->tm_mday = 1;
	tm->tm_mon = 0;

	if (isdigit(optarg[0]) &&
	    isdigit(optarg[1]) &&
	    isdigit(optarg[2]) &&
	    isdigit(optarg[3]) &&
	    (!optarg[4] || optarg[4] == '-')) {
		tm->tm_year = atoi(optarg) - 1900;

		if (!optarg[4]) {
			flagy = 1;
		} else if (isdigit(optarg[5]) && isdigit(optarg[6]) &&
		    (!optarg[7] || optarg[7] == '-')) {
			tm->tm_mon = atoi(optarg+5) - 1;

			if (isdigit(optarg[8]) && isdigit(optarg[9]) &&
			    (!optarg[10] || optarg[10] == '-'))
				tm->tm_mday = atoi(optarg+8);
		}
	}

	mktime(tm);
}

int
main(int argc, char *argv[])
{
	time_t now = time(0);
	struct tm *tm = gmtime(&now);

	int c;
	while ((c = getopt(argc, argv, "13cid:y")) != -1)
		switch (c) {
		case '1': flag1 = 1; break;
		case '3': flag3 = 1; break;
		case 'c': flagc = 1; break;
		case 'y': flagy = 1; break;
		case 'i': flagi = 1; break;
		case 'd': parse_isodate(optarg, tm); break;
		}

	tm->tm_hour = tm->tm_min = tm->tm_sec = 0;

	int today_mday = tm->tm_mday;
	int today_mon = tm->tm_mon;
	int today_year = tm->tm_year;

	int max_weeks = 1;
	int max_months = 0;

	if (flagc) {
		max_weeks = 1;
	} else if (flag3) {
		tm->tm_mday = 1; tm->tm_mon--; max_weeks = 14;
	} else if (flagy) {
	        tm->tm_mday = 1; tm->tm_mon = 0; max_months = 12; max_weeks = 53;
        } else if (flagi) {
		tm->tm_mday = 1;
	} else { /* flag1 is default */
	        tm->tm_mday = 1; max_months = 1; max_weeks = 6;
	}

	mktime(tm);

	while (tm->tm_wday != 1) {
		tm->tm_mday--;
		mktime(tm);
	}

	int color = isatty(1);

	printf("        %sMo Tu We Th Fr Sa Su%s\n",
	    color ? "\e[4m" : "",
	    color ? "\e[0m" : "");

	for (int weeks = 0, months = 0;
	     flagi || (max_months && months < max_months) || (weeks < max_weeks);
	     weeks++) {
		char buf[8];
		strftime(buf, sizeof buf, "%V", tm);
		printf("%s ", buf);

		tm->tm_mday += 6;
		mktime(tm);
		if (tm->tm_mday <= 7 || weeks == 0) {
			strftime(buf, sizeof buf, "%b", tm);
			printf("%s ", buf);
			months++;
		} else if ((tm->tm_mon == 0 && tm->tm_yday < 14) || weeks == 1) {
			printf("%04d", tm->tm_year + 1900);
		} else {
			printf("    ");
		}
		tm->tm_mday -= 6;
		mktime(tm);

		for (int i = 0; i < 7; i++) {
			int today =
			    tm->tm_mday == today_mday &&
			    tm->tm_mon == today_mon &&
			    tm->tm_year == today_year;

			printf(" %s%2d%s",
			    color && today ? "\e[7m" : "",
			    tm->tm_mday,
			    color && today ? "\e[0m" : "");

			tm->tm_mday++;
			mktime(tm);
		}

		printf("\n");
	}
}
