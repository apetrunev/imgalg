./img-circle -l60 -h180 --rmin 375 --rmax 380 --step 1 -f image.png > edge-points.txt
./center -f edge-pixels.txt image.png

./img-circle -l40 -h120 --rmin 18 --rmax 18 --step 1 -f image.png > edge-points.txt
./center -f edge-pixels.txt image.png
