These are various tools to examine/plot the SBS-1 format output from
a Raspberry Pi ADS-B receiver/feeder for Flightradar24.  I use these to
evaluate the effectiveness and range of various antenna configurations.

1. Modify basecoords.h to reflect the location of your antenna then:

        $ make

2. Capture some input:

        $ wget -O - -q http://myreceiver:30003 > log

3. Do some processing:

        $ sbs1view < log | egrep -v '^MSG,[458],' > log.filtered

4. Sort reported positions by distance:

        $ fgrep '#' log.filtered | sort -n -t '#' -k 2 | less

5. Plot positions with gnuplot:

        $ sbs1tognuplot testplot < log.filtered
        $ gnuplot
        plot 'testplot-points.dat' using 1:2, 'circle.dat' using 2:1:3 with circles lt rgb 'black'

6. Produce a KML file for display in Google Earth:

        $ sbs1tokml < log.filtered > positions.kml
