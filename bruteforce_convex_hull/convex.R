#! /usr/bin/env Rscript
png("convex.png", width=700, height=700)
plot(1:10000, 1:10000, type="n")

#points
points(190,1634)
points(9983,8105)
points(7286,9308)
points(3819,4375)
points(2053,8783)
points(8871,5009)
points(6283,3801)
points(5455,1286)
points(9973,9839)
points(1567,4811)

#line segments
segments(190,1634,2053,8783)
segments(190,1634,5455,1286)
segments(9983,8105,8871,5009)
segments(9983,8105,9973,9839)
segments(2053,8783,9973,9839)
segments(8871,5009,5455,1286)
dev.off()
