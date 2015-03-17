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

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/*::  This function converts radians to decimal degrees             : */
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
double
rad2deg(double rad)
{
	return (rad * 180.0 / M_PI);
}

static double
CalcDistance(const coord_t * a, const coord_t * b)
{

	double lat1, lon1, lat2, lon2;
	char unit;
	double theta, dist;

	lat1 = a->lat;
	lon1 = a->lng;
	lat2 = b->lat;
	lon2 = b->lng;
	unit = 'K';

	theta = lon1 - lon2;
	dist = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(theta);
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	switch (unit)
	{
	case 'M':
		break;
	default:
	case 'K':
		dist = dist * 1.609344;
		break;
	case 'N':
		dist = dist * 0.8684;
		break;
	}
	return dist;
}

static void
CoordsDegreesToRadians(coord_t * a)
{
	a->lat = DegreesToRadians(a->lat);
	a->lng = DegreesToRadians(a->lng);
}

static int
ProcessMsg6(const char buf[512], char notes[512], int *altitude)
{
	int i, comma_count;

	// MSG,6,333,11345,ABDDC4,11445,2015/03/02,16:00:30.564,2015/03/02,16:00:30.564,,15025,,,,,,7743,0,0,0,0
	strcpy(notes, "bad MSG,6!");
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
	sscanf(&buf[i], "%d,", altitude);
	notes[0] = '\0';

	return 0;
}

static int
ProcessMsg3(const coord_t * base_coords, const char buf[512], char notes[512], double *distance, int *altitude)
{
	int i, comma_count;
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
	sscanf(&buf[i], "%d,", altitude);
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
	int c, verbose, errflag, len, distance_limit, suppress, altitude, altitude_limit;
	char buf[512], notes[512];
	double best_distance, distance;
	static coord_t base_coords = { BASE_LAT, BASE_LONG };

	best_distance = 0;
	verbose = 0;
	errflag = 0;
	distance_limit = -1;
	altitude_limit = -1;
	while ((c = getopt(argc, argv, "a:vd:")) != EOF)
		switch (c)
		{
		case 'v':
			++verbose;
			break;
		case 'd':
			distance_limit = strtoul(optarg, 0, 0);
			break;
		case 'a' :
			altitude_limit =  strtoul(optarg, 0, 0);
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
		strtok(buf, " #");
		notes[0] = '\0';
		suppress = 0;
		if (strncmp(buf, "MSG,3,", 6) == 0)
		{
			if (ProcessMsg3((const coord_t *)&base_coords, buf, notes, &distance, &altitude) >= 0)
			{
				suppress = (distance_limit > 0 && distance > distance_limit) || (altitude_limit > 0 && altitude >= altitude_limit);
				if (!suppress && distance >= best_distance)
				{
					best_distance = distance;
					strcat(notes, " (best!)");
				}
			}
		}
		else if (strncmp(buf, "MSG,6,", 6) == 0)
			if (ProcessMsg6(buf, notes, &altitude) >= 0)
				suppress = altitude_limit > 0 && altitude >= altitude_limit;

		if (!suppress)
			printf("%-130s %c %s\n", buf, notes[0] ? '#' : ' ', notes);
		fflush(stdout);
	}

	return 0;
}
