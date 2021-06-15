# PPM Image Data Processing
This program is about manipuling an image in PPM format.
The format is simple and straightforward and if you want to practice bitwise manipulation  this is a good environment.

To give you a glimpse on how a "plain" PPM Format, this is how it looks like.
```
P3 # P3 is a magic number 
# use sharp to comment. similar to // 
4 4 # height and width
15 # Maximum color value
 0  0  0    0  0  0    0  0  0   15  0 15
 0  0  0    0 15  7    0  0  0    0  0  0
 0  0  0    0  0  0    0 15  7    0  0  0
15  0 15    0  0  0    0  0  0    0  0  0
```

This program though accepts only raw PPM format (P6 is the magic number).
If you want to see the the document of PPM, you can visit [here](http://netpbm.sourceforge.net/doc/ppm.html)
## How to use

To use the program. you can start by running ppmx.exe
```
$ ./ppmx

Usage: ppmx [options] (filename.ppm)
Options:
-fv             Flip vertically
-fh             Flip horizontally
-w<width>       Scale to the new width (0 - 9999)
-r<angle>       Rotate CW (0 - 359)
-mono           Convert to bilevel (.pbm)format
-gray           Convert to grayscale (.pgm) format
```

If don't trust my .exe file (you should be!), you can copy my code and compile it on your own.

## Commands
1.   -fv: flip vertically
2.   -fh: flip horizontally
3. -w(n): Scale to n width (-w100 means new width is 100)
4. -r(θ): rotate in couter clockwise. (-r30 rotate 30 degrees in CW) 
5. -mono: Convert to Bilevel (.pbm) format
6. -gray: Convert to grayscale (.pgm) format

Commands can be specified in any order but no duplication of commands.


(Images below are in PNG format since Github doesn't support PPM. This is just for showing the output)

Example 1: ppmx  -w1080 -mono image-data.ppm
![ocean pbm](https://user-images.githubusercontent.com/28287818/122006755-cb7d6980-cde9-11eb-88f8-5924ceed7c9d.png)

Example 2: ppmx -fv -r45 -w720 image-data.ppm
![ocean ppm](https://user-images.githubusercontent.com/28287818/122007662-c371f980-cdea-11eb-9eee-e9fdcf3221da.png)

Example 3: ppmx -gray -fh image-data.ppm
![ocean pgm](https://user-images.githubusercontent.com/28287818/122008221-5d39a680-cdeb-11eb-8e9f-26744a58209f.png)

### PS
 This program was written when I was still a fresh graduate (2018). There are a lot of things that needs to be changed like proper naming of variables, ineffecient code, fix potential bugs, etc. I don't have plans to update the source code since it is already a bit unreadable to me 😅. This will just be my reference and see how much I improve since I started my career.

# References
PPM: http://netpbm.sourceforge.net/doc/ppm.html (color image format)

PGM: http://netpbm.sourceforge.net/doc/pgm.html (grayscale image format)

PBM: http://netpbm.sourceforge.net/doc/pbm.html (Bi-level image format)

Program's Specification
1. [2018-MidYear-ProgramingContest.pdf](https://github.com/pkdimpas/PPM-Image-Processing/files/6653613/2018-MidYear-ProgramingContest.pdf)
