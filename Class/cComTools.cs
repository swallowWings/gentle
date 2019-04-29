using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace gentle
{
   public class cComTools
    {
        public static void timeDelay()
        {
            int tmp = 0;
            for (int a = 0; a < 10; a++)
            {
                tmp += 1;
            }// time delay
        }

        public static string GetTimeToPrintOut(bool bDateTimeFormat, string startDateTime, int nowT_MIN_elapsed)
        {
            if (bDateTimeFormat == true)
            {
                string tv = Convert.ToDateTime(startDateTime).Add(new System.TimeSpan(0, nowT_MIN_elapsed, 0)).ToString("yyyy/MM/dd HH:mm");
                return tv;
            }
            else
            {
                return string.Format(((double) nowT_MIN_elapsed / 60).ToString("F2"));
            }
        }

        public static string GetTimeToPrintOut(bool bDateTimeFormat, string startDateTime, int nowT_MIN_elapsed, string stringFormat)
        {
            if (bDateTimeFormat == true)
            {
                return string.Format(Convert.ToDateTime(startDateTime).Add(new System.TimeSpan(0, nowT_MIN_elapsed, 0)).ToString (),
                    stringFormat);
            }
            else
            {
                return string.Format((nowT_MIN_elapsed / 60).ToString("F"));
            }
        }

        public static string GetTimeStringFromDateTimeFormat(string nowTimeToPrintOut)
        {
            nowTimeToPrintOut = nowTimeToPrintOut.Replace("/", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( " ", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( ":", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( "-", "");

            return nowTimeToPrintOut;
        }


        public static string GetNowTimeToPrintOut(string strStartingTime, int TimeStepToOut_MIN, int intNowOrder)
        {
            string strNowTimeToPrintOut = null;
            try
            {
                //strStartingTime = strStartingTime.Replace("/", "");
                //strStartingTime = strStartingTime.Replace(" ", "");
                //strStartingTime = strStartingTime.Replace(":", "");
                //strStartingTime = strStartingTime.Replace("-", "");
                strNowTimeToPrintOut = string.Format(Convert.ToDateTime(strStartingTime).Add(new System.TimeSpan(0, TimeStepToOut_MIN * intNowOrder, 0)).ToString("yyyyMMddHHmm"));
                return strNowTimeToPrintOut;
            }
            catch (Exception ex)
            {
                return "-1";
                throw ex;
            }
        }


        public static bool IsNumeric(string value)
        {
            float v = 0;
            return float.TryParse(value, out v);
        }


        /// <summary>
        /// 문자열에 공백 있을 경우 필수 사용
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public static string SetDQ(string str)
        {
            string result = "";
            result = "\"" + str + "\"";
            return result;
        }

        public static long GetNumericRoundDown(double dblInputValue)
        {
            int intPointPosition = 0;
            intPointPosition = dblInputValue.ToString ().IndexOf (".");
            if (intPointPosition == 0)
            {
                return Convert.ToInt64(dblInputValue);
            }
            else
            {
                return Convert.ToInt64(Convert.ToString(dblInputValue).Substring(0, intPointPosition));
            }
        }

 

        public static List<string> GetListFromDataTable(DataTable indt, int cidx)
        {
            List<string> nl = new List<string>();
            for (int nr = 0; nr < indt.Rows.Count; nr++)
            {
                nl.Add(indt.Rows[nr].ItemArray[cidx].ToString());
            }
            return nl;
        }

        public static List<string> GetTimeListToPrintout(string rainfallStartDateTime, int TimeInterval_Min, int ListCountToSet)
        {
            List<string> l = new List<string>();
            if (ListCountToSet > 1)
            {
                for (int n = 0; n < ListCountToSet; n++)
                {
                    int travel_min = n * TimeInterval_Min;
                    string T = Convert.ToDateTime(rainfallStartDateTime).Add(new System.TimeSpan(0, travel_min, 0)).ToString("yyyy/MM/dd HH:mm") ;
                    l.Add(T);
                }
            }
            return l;
        }

        public static void ClearTextInControl(params Control[] targets)
        {
            foreach (Control c in targets)
            {
                c.Text = "";
            }
        }

        public static string getTailFromString(string strIn, string strSeparator)
        {
            int pos1 = 0;
            pos1 = strIn.IndexOf(strSeparator);
            return strIn.Substring(pos1 + 1).Trim();
        }

        public static string getHeadFromString(string strIn, string strSeparator)
        {
            int pos1 = 0;
            pos1 = strIn.IndexOf(strSeparator);
            return strIn.Substring(0, pos1 + 1).Trim();
        }
    }

    public class NaturalComparer : Comparer<string>, IDisposable
    {
        private Dictionary<string, string[]> table;

        public NaturalComparer()
        {
            table = new Dictionary<string, string[]>();
        }

        public void Dispose()
        {
            table.Clear();
            table = null;
        }

        public override int Compare(string x, string y)
        {
            if (x == y)
            {
                return 0;
            }
            string[] x1, y1;
            if (!table.TryGetValue(x, out x1))
            {
                x1 = Regex.Split(x.Replace(" ", ""), "([0-9]+)");
                table.Add(x, x1);
            }
            if (!table.TryGetValue(y, out y1))
            {
                y1 = Regex.Split(y.Replace(" ", ""), "([0-9]+)");
                table.Add(y, y1);
            }

            for (int i = 0; i < x1.Length && i < y1.Length; i++)
            {
                if (x1[i] != y1[i])
                {
                    return PartCompare(x1[i], y1[i]);
                }
            }
            if (y1.Length > x1.Length)
            {
                return 1;
            }
            else if (x1.Length > y1.Length)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }

        private static int PartCompare(string left, string right)
        {
            int x, y;
            if (!int.TryParse(left, out x))
            {
                return left.CompareTo(right);
            }

            if (!int.TryParse(right, out y))
            {
                return left.CompareTo(right);
            }

            return x.CompareTo(y);
        }
    }

    //public class NaturalComparer : IComparer<string>
    //{
    //    private int pos;
    //    private readonly int ordr;

    //    public NaturalComparer(bool Ascending = true)
    //    {
    //        ordr = Ascending ? 1 : -1;
    //    }

    //    private static string[] RegexSplit(string s)
    //    {
    //        return Regex.Split(s, @"(\d+)", RegexOptions.IgnoreCase);
    //    }

    //    private static Predicate<string> GetEmptyStrings()
    //    {
    //        return s => string.IsNullOrEmpty(s);
    //    }

    //    public int Compare(string x, string y)
    //    {
    //        List<string> left = new List<string>(RegexSplit(x));
    //        List<string> right = new List<string>(RegexSplit(y));

    //        left.RemoveAll(GetEmptyStrings());
    //        right.RemoveAll(GetEmptyStrings());

    //        pos = 0;
    //        foreach (string e in left)
    //        {
    //            if (y.Count() > pos)
    //            {
    //                decimal ov;
    //                if (!decimal.TryParse(x, out ov) && !decimal.TryParse(right[pos], out ov))
    //                {
    //                    int result = string.Compare(x, right[pos], true);
    //                    if (result != 0)
    //                        return result * ordr;
    //                    else
    //                        pos += 1;
    //                }
    //                else if (decimal.TryParse(x, out ov) && !decimal.TryParse(right[pos], out ov))
    //                    return -1 * ordr;
    //                else if (!decimal.TryParse(x, out ov) && decimal.TryParse(right[pos], out ov))
    //                    return 1 * ordr;
    //                else
    //                {
    //                    var result = decimal.Compare(decimal.Parse(x), decimal.Parse(right[pos]));
    //                    if (result == 0)
    //                        pos += 1;
    //                    else
    //                        return result * ordr;
    //                }
    //            }
    //            else
    //                return -1 * ordr;
    //        }

    //        return ordr;
    //    }
    //}
}
