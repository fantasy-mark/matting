from PIL import Image
import numpy

def my_open_jpg(filename):
    "open a jpg file and return a file object"
    return Image.open(filename);

def my_jpg2yuv(im_jpg):
    "conver a jpg file to yuv file"
    #if(len(im_jpg.split())==4):
    #    r,g,b,a=im_jpg.split();
    #else:
    r,g,b=im_jpg.split();

    width,height=im_jpg.size;
    im_new=range(width*height*3/2);
    k_r = 0.299;
    k_g = 0.587
    k_b = 0.114
    r = list(r.getdata());
    g = list(g.getdata());
    b = list(b.getdata());
    for i in range(height):
        for j in range(width):
            index = i*width+j;
            R = r[index];
            G = g[index];
            B = b[index];
            Y = k_r*R + k_g*G + k_b*B;
            if (not(i&0x01)and not(j&0x01)):
                im_new[width*height+i/2*width/2+j/2] = (B-Y)/2/(1-k_b)+128;
                im_new[width*height*5/4+i/2*width/2+j/2] = (R-Y)/2/(1-k_r)+128;

            im_new[index]=Y;

    return im_new;

def my_save_yuv(filename,im_new):
    #use the numpy to write the data to file
    fp = open(filename,"wb");

    data=numpy.array(im_new,"B");
    data.tofile(fp);
    fp.close();
    print "save yuv file %s successfully" % filename

if __name__ == "__main__":
    im = my_open_jpg("image0.jpg");
    print im;
    im_yuv = my_jpg2yuv(im);
    print im_yuv
    my_save_yuv("image0.yuv",im_yuv);
    print "end";

