#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#include "basecoords.h"

typedef struct coord_t
{
	double lat;
	double lng;
} coord_t;

static double
DegreesToRadians(double d)
{
	double r;

	r = (d * M_PI) / 180.0;

	return r;
}

static double
CalcDistance(const coord_t * a, const coord_t * b)
{
	double x, y, d;
	static const double r = 6371;	// earth radius km

	x = (a->lng - b->lng) * cos((a->lat + b->lat) / 2.0);
	y = b->lat - a->lat;
	d = sqrt(x * x + y * y) * r;

	return d;
}

static void
CoordsDegreesToRadians(coord_t * a)
{
	a->lat = DegreesToRadians(a->lat);
	a->lng = DegreesToRadians(a->lng);
}

static int
ProcessMsg3(const coord_t * base_coords, const char buf[512], char notes[512], double *distance)
{
	int i, altitude, comma_count;
	coord_t remote_coords;

	// MSG,3,333,3560,A5DBE5,3660,2015/02/26,14:01:33.635,2015/02/26,14:01:33.635,,8850,,,34.20128,-118.44160,,,0,0,0,0
	i = 0;
	comma_count = 0;
	strcpy(notes, "bad MSG,3!");
	while (buf[i] != 0 && comma_count < 11)
	{
		comma_count += buf[i] == ',';
		++i;
	}
	if (comma_count < 11)
		return -1;
	sscanf(&buf[i], "%d,", &altitude);
	++i;
	while (buf[i] != 0 && comma_count < 14)
	{
		comma_count += buf[i] == ',';
		++i;
	}
	if (comma_count < 14)
		return -1;
	sscanf(&buf[i], "%lf,%lf,", &remote_coords.lat, &remote_coords.lng);
	CoordsDegreesToRadians(&remote_coords);
	*distance = CalcDistance(base_coords, &remote_coords);
	sprintf(notes, "%f km", *distance);

	return 0;
}

int
main(int argc, char *argv[])
{
	int c, verbose, errflag, len, distance_limit, suppress;
	char buf[512], notes[512];
	double best_distance, distance;
	static coord_t base_coords = { BASE_LAT, BASE_LONG };

	best_distance = 0;
	verbose = 0;
	errflag = 0;
	distance_limit = -1;
	while ((c = getopt(argc, argv, "vd:")) != EOF)
		switch (c)
		{
		case 'v':
			++verbose;
			break;
		case 'd':
			distance_limit = strtoul(optarg, 0, 0);
			break;
		default:
			errflag = 1;
			break;
		}
	if (!errflag && argc - optind == 2)
	{
		sscanf(argv[optind], "%lf", &base_coords.lat);
		sscanf(argv[optind + 1], "%lf", &base_coords.lng);
	}

	if (errflag)
	{
		fprintf(stderr, "usage: %s [-v] baselat baselong (default %f %f)\n", argv[0], base_coords.lat, base_coords.lng);
		exit(1);
	}
	CoordsDegreesToRadians(&base_coords);

	while (fgets(buf, sizeof(buf), stdin) && (len = strlen(buf)) > 2)
	{
		buf[len - 2] = '\0';
		notes[0] = '\0';
		suppress = 0;
		if (strncmp(buf, "MSG,3,", 6) == 0)
			if (ProcessMsg3((const coord_t *)&base_coords, buf, notes, &distance) >= 0)
			{
				suppress = distance_limit > 0 && distance >= distance_limit;
				if (!suppress && distance >= best_distance)
				{
					best_distance = distance;
					strcat(notes, " (best!)");
				}
			}

		if (!suppress)
			printf("%-130s %c %s\n", buf, notes[0] ? '#' : ' ', notes);
		fflush(stdout);
	}

	return 0;
}
