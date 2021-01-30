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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int flag1, flag3, flagc, flagC, flagi, flagy;

const char monthname[13][4] = {
	"   ",
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char wdayname[8][3] = {
	"  ",
	"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"
};

/* all args are 1-indexed */
long
ymd2jd(int year, int month, int day)
{
	month--;
	if (month >= 12 || month < 0) {
		int adj = month / 12;
		month %= 12;
		if (month < 0) {
			adj--;
			month += 12;
		}
		year += adj;
	}
	month++;

	if (month < 3) {
		year--;
		month += 12;
	}
	year += 4800;
	return day + (153*(month-3)+2)/5 +
	    365*year + year/4 - year/100 + year/400 - 32045;
}

/* Mon = 1, Sun = 7 */
void
jd2ymdwi(long jd, int *year, int *month, int *day, int *wday, int *isoweek)
{
	int _;
	long e = 4*(jd+1401+(4*jd+274277)/146097*3/4-38) + 3;
	long h = e%1461/4*5 + 2;

	*day = h%153/5 + 1;
	int m = *month = (h/153+2)%12 + 1;
	int y = *year = e/1461 - 4716 + (14-m)/12;
	int w = *wday = jd%7 + 1;

	int yday = jd - ymd2jd(y, 1, 1) + 1;
	*isoweek = (yday - w + 10) / 7;
	if (*isoweek < 1)
		jd2ymdwi(ymd2jd(y - 1, 12, 31), &_, &_, &_, &_, isoweek);
	else if (*isoweek == 53 &&
	    ymd2jd(y+1, 1, 1) + (7 - (y+y/4-y/100+y/400+3)%7 - 4) <= jd)
		*isoweek = 1;
}

void
parse_isodate(char *optarg, int *y, int *m, int *d)
{
	*m = *d = 1;

	if (isdigit(optarg[0]) &&
	    isdigit(optarg[1]) &&
	    isdigit(optarg[2]) &&
	    isdigit(optarg[3]) &&
	    (!optarg[4] || optarg[4] == '-')) {
		*y = atoi(optarg);

		if (!optarg[4]) {
			flagy = 1;
		} else if (isdigit(optarg[5]) && isdigit(optarg[6]) &&
		    (!optarg[7] || optarg[7] == '-')) {
			*m = atoi(optarg+5);

			if (isdigit(optarg[8]) && isdigit(optarg[9]) &&
			    (!optarg[10] || optarg[10] == '-'))
				*d = atoi(optarg+8);
		}
	}
}

int
main(int argc, char *argv[])
{
	time_t now = time(0);
	struct tm *tm = localtime(&now);

	int y = tm->tm_year + 1900;
	int m = tm->tm_mon + 1;
	int d = tm->tm_mday;
	int w;
	int i;
	int cw;
	int ci;

	int c;
	while ((c = getopt(argc, argv, "13cCid:y")) != -1)
		switch (c) {
		case '1': flag1 = 1; break;
		case '3': flag3 = 1; break;
		case 'c': flagc = 1; break;
		case 'C': flagC = 1; break;
		case 'y': flagy = 1; break;
		case 'i': flagi = 1; break;
		case 'd': parse_isodate(optarg, &y, &m, &d); break;
		case '?': exit(1);
		}

	long today = ymd2jd(y, m, d);
	jd2ymdwi(today, &y, &m, &d, &cw, &ci);

	int max_weeks = 1;
	int max_months = 0;

	if (flagc) {
		max_weeks = 1;
	} else if (flag3) {
		d = 1; m--; max_weeks = 14;
	} else if (flagy) {
		d = 1; m = 1; max_months = 12; max_weeks = 53;
	} else if (flagi) {
		d = 1;
	} else { /* flag1 is default */
		d = 1; max_months = 1; max_weeks = 6;
	}

	long jd = ymd2jd(y, m, d);

	jd2ymdwi(jd, &y, &m, &d, &w, &i);
	jd -= (w - 1);   // skip back to last monday.
	jd2ymdwi(jd, &y, &m, &d, &w, &i);

	int color = isatty(1) || flagC;

	printf("        ");
	if (color)
		printf("\e[4m");
	for (int wd = 1; wd <= 7; wd++) {
		if (color && wd == cw)
			printf("\e[7m");
		printf("%s", wdayname[wd]);
		if (color && wd == cw)
			printf("\e[27m");
		if (wd < 7)
			printf(" ");
	}
	if (color)
		printf("\e[0m");
	printf("\n");

	for (int weeks = 0, months = 0;
	     flagi || (max_months && months < max_months) || (weeks < max_weeks);
	     weeks++) {
		if (color && i == ci)
			printf("\e[7m");
		printf("%02d", i);
		if (color && i == ci)
			printf("\e[27m");
		printf(" ");

		int ey, em, ed, ew, ei;
		jd2ymdwi(jd + 6, &ey, &em, &ed, &ew, &ei);   /* end of week */

		if (ed <= 7 || weeks == 0) {
			printf("%s ", monthname[em]);
			months++;
		} else if ((em == 1 && ed <= 14) || weeks == 1) {
			printf("%04d", ey);
		} else {
			printf("    ");
		}

		do {
			printf(" %s%2d%s",
			    color && (jd == today) ? "\e[7m" : "",
			    d,
			    color && (jd == today) ? "\e[27m" : "");

			jd++;
			jd2ymdwi(jd, &y, &m, &d, &w, &i);
		} while (w != 1);

		printf("\n");
	}
}
