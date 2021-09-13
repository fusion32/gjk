# GJK Implementation

## License: `LICENSE.txt` (Public Domain)

## About
This is a simple implementation of the GJK algorithm based mostly on Casey Muratori's video from 2006 (https://www.youtube.com/watch?v=Qupqu1xe7Io).

There are two core functions `gjk_collision_test` and `gjk`. The first is the version from the video which only tests for overlaps. The second is more complete and will return the distance, closest points, and closest features from the polygons. 

Note that I say polygons above because I ended up simplifying the prototype of the functions to accept two polygons instead of two generic shapes and an appropriate support function. `gjk(polygon1, polygon2)` instead of `gjk(shape1, shape2, support_funtion)`.

## Why
While working on a personal project, I didn't find any implementation that was readable enough for me to take notes. So naturally I had to do some digging before coming up with this version. This isn't the best version you'll see out there but it will do the job.

## Compiling
Run the `build.bat` script with the Visual Studio Shell. The only dependency is SDL2. Set the system/user variable SDL_PATH to point to SDL's path or edit `build.bat` and change the line:
```
if [%SDL_PATH%]==[] @SET SDL_PATH="X:/libs/SDL/2.0.14"
```

For Linux, since `build.bat` is only a couple of lines, it shouldn't be a problem converting it to a bash script.

## Known Issues
- In cases where two faces are parallel (and the polygons are not overlapping), the closest points can flicker if the polygons are moving. This is because there is a range of solutions in this problem. I've added some NOTEs and TODOs in `gjk.cc` mentioning it but I haven't done anything to try to "fix" this.
