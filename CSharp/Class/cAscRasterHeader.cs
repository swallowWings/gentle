using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
   public class cAscRasterHeader
    {
        public  int numberCols;
        public int numberRows;
        public double xllcorner;
        public double yllcorner;
        public double cellsize;
        public double dx;
        public double dy;
        public int nodataValue;
        public int headerEndingLineIndex;
        public int dataStartingLineIndex;
    }

    public class cRasterExtent
    {
        public  double bottom;
        public double top;
        public double left;
        public double right;
        public double extentWidth;
        public double extentHeight;

        public cRasterExtent(cAscRasterHeader header)
        {
            bottom = header.yllcorner;
            if (header.cellsize>0)
            {
                top = header.yllcorner + header.numberRows * header.cellsize;
            }
            else
            {
                top = header.yllcorner + header.numberRows * header.dy;
            }
            left= header.xllcorner;
            if (header.cellsize > 0)
            {
                right = header.xllcorner + header.numberCols * header.cellsize;
            }
            else
            {
                right = header.xllcorner + header.numberCols * header.dx;
            }
            extentWidth = right - left;
            extentHeight = top - bottom;
        }
    }
}
