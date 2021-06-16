using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Data;

namespace gentle
{
    public class cTextFile
    {
        private const int BigSizeThreshold = 70000000;//7천만개 기준 //200000000;//2억개 기준
        public enum ValueSeparator
        {
            COMMA,
            SPACE,
            TAB,
            FORWARD_SLASH,
            ALL,
            NONE
        };

        //public static bool ConfirmDeleteFiles(List<string> FilePathNames)
        //{
        //    bool bAlldeleted = false;
        //    int n = 0;
        //    while (!(bAlldeleted == true))
        //    {
        //        n += 1;
        //        foreach (string fpn in FilePathNames)
        //        {
        //            if (File.Exists(fpn) == true)
        //            {
        //                File.Delete(fpn);
        //            }
        //        }
        //        foreach (string fpn in FilePathNames)
        //        {
        //            if (File.Exists(fpn) == false)
        //            {
        //                bAlldeleted = true;
        //            }
        //            else
        //            {
        //                bAlldeleted = false;
        //                break; 
        //            }
        //        }
        //        if (n > 100)
        //            return false;
        //    }
        //    return true;
        //}

        //public static bool ConfirmDeleteFiles(string FilePathNames)
        //{
        //    bool bAlldeleted = false;
        //    int n = 0;
        //    while (!(bAlldeleted == true))
        //    {
        //        n += 1;
        //        if (File.Exists(FilePathNames))
        //        {
        //            File.Delete(FilePathNames);
        //        }
        //        if (File.Exists(FilePathNames) == false)
        //        {
        //            bAlldeleted = true;
        //        }
        //        else
        //        {
        //            bAlldeleted = false;
        //        }
        //        if (n > 100)
        //            return false;
        //    }
        //    return true;
        //}

        public static string MakeHeaderString(int ncols, int nrows, double xll, double yll, double cellSize, double dx, double dy, string nodataValue)
        {
            string header = "";
            header = header + "ncols" + " " + Convert.ToString(ncols) + "\r\n";
            header = header + "nrows" + " " + Convert.ToString(nrows) + "\r\n";
            header = header + "xllcorner" + " " + Convert.ToString(xll) + "\r\n";
            header = header + "yllcorner" + " " + Convert.ToString(yll) + "\r\n";
            if (dx!=dy && dx>0 && dy>0)
            {
                header = header + "dx" + " " + Convert.ToString(dx) + "\r\n";
                header = header + "dy" + " " + Convert.ToString(dy) + "\r\n";
            }
            else
            {
                header = header + "cellsize" + " " + Convert.ToString(cellSize) + "\r\n";
            }            
            header = header + "NODATA_value" + " " + nodataValue + "\r\n";
            return header;
        }

        public static bool MakeASCTextFile(string fpn, int ncols, int nrows, double xll, double yll, double cellSize, string nodataValue, string[] rowsArray)
        {
            if (File.Exists(fpn) == true) { File.Delete(fpn); }
            string header = cTextFile.MakeHeaderString(ncols, nrows, xll, yll, cellSize, 0,0, nodataValue);
            File.AppendAllText(fpn, header);
            for (int n = 0; n < rowsArray.Length; n++)
            {
                File.AppendAllText(fpn, rowsArray[n] + "\r\n");
            }
            return true;
        }

        public static bool MakeASCTextFile(string fpn, string allHeader, string[] strArray)
        {
            if (File.Exists(fpn) == true)
            {
                File.Delete(fpn);
            }
            if (File.Exists(fpn) == false)
            {
                File.AppendAllText(fpn, allHeader);
                for (int n = 0; n < strArray.Length; n++)
                {
                    File.AppendAllText(fpn, strArray[n] + "\r\n");
                }
            }
            return true;
        }

        public static bool MakeASCTextFile(string fpn, int ncols, int nrows, double xll, double yll, float cellSize, int nodataValue, double[,] array, int decimalPartN)
        {
            if (File.Exists(fpn) == true) { File.Delete(fpn); }
            string header = cTextFile.MakeHeaderString(ncols, nrows, xll, yll, cellSize, 0,0, nodataValue.ToString());
            File.AppendAllText(fpn, header);
            WriteTwoDimData(fpn, array, decimalPartN, nodataValue);
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="fpn"></param>
        /// <param name="allHeader">모든 헤더. nodata 부분까지 포함</param>
        /// <param name="array">래스터 값을 저장하고 있는 2차원 배열</param>
        /// <param name="decimalPartN">소숫점 이하 출력 자리 수</param>
        /// <returns></returns>
        public static bool MakeASCTextFile(string fpn, string allHeader, double[,] array, int decimalPartN, int nodataValue)
        {
            if (File.Exists(fpn) == true)
            {
                File.Delete(fpn);
            }
            if (File.Exists (fpn)==false )
            {
                File.AppendAllText(fpn, allHeader);
                WriteTwoDimData(fpn, array, decimalPartN, nodataValue);
            }
            return true;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="fpn"></param>
        /// <param name="array"></param>
        /// <param name="decimalPartNum">0~7 이외의 숫자가 들어오면, 원본 값 그대로 저장</param>
        private static void WriteTwoDimData(string fpn, double[,] array, int decimalPartNum, int nodataValue)
        {
            string dpn = "";
            if (decimalPartNum == 0) { dpn = "F0"; }
            else if (decimalPartNum == 1) { dpn = "F1"; }
            else if (decimalPartNum == 2) { dpn = "F2"; }
            else if (decimalPartNum == 3) { dpn = "F3"; }
            else if (decimalPartNum == 4) { dpn = "F4"; }
            else if (decimalPartNum == 5) { dpn = "F5"; }
            else if (decimalPartNum == 6) { dpn = "F6"; }
            else if (decimalPartNum == 7) { dpn = "F7"; }
            int nx = array.GetLength(0);
            int ny = array.GetLength(1);
            bool isBigSize = false;
            if (nx * ny > BigSizeThreshold) { isBigSize = true; }

            if (isBigSize==false )
            {
                StringBuilder sbALL = new StringBuilder("");
                for (int nr = 0; nr < ny; nr++)
                {
                    for (int nc = 0; nc < nx; nc++)
                    {
                        if (array[nc, nr] == 0 || array[nc, nr] == nodataValue)
                        {
                            sbALL.Append(array[nc, nr].ToString() + " ");
                        }
                        else
                        {
                            sbALL.Append(array[nc, nr].ToString(dpn) + " ");
                        }
                    }
                    sbALL.Append("\r\n");
                }
                File.AppendAllText(fpn, sbALL.ToString());
            }
            else
            {
                for (int nr = 0; nr < ny; nr++)
                {
                    StringBuilder sbArow = new StringBuilder();
                    for (int nc = 0; nc < nx; nc++)
                    {
                        if (array[nc, nr] == 0 || array[nc, nr] == nodataValue)
                        {
                            sbArow.Append(array[nc, nr].ToString() + " ");
                        }
                        else
                        {
                            sbArow.Append(array[nc, nr].ToString(dpn) + " ");
                        }
                    }
                    sbArow.Append("\r\n");
                    File.AppendAllText(fpn, sbArow.ToString());
                    if (nr % 300 == 0) { GC.Collect(); }
                }
            }
        }

        private static void WriteTwoDimData_old_201810(string fpn, double[,] array)
        {
            StringBuilder sbALL = new StringBuilder("");
            for (int nr = 0; nr < array.GetLength(1); nr++)
            {
                for (int nc = 0; nc < array.GetLength(0); nc++)
                {
                    sbALL.Append(array[nc, nr].ToString("F2"));
                    sbALL.Append(" ");
                }
                sbALL.Append("\r\n");
            }
            File.AppendAllText(fpn, sbALL.ToString());
        }

        private static void WriteTwoDimData_old_v20180627(string fpn, double [,] array)
        {
            string rows = "";
            for (int nr = 0; nr < array.GetLength(1); nr++)
            {
                string arow = "";
                for (int nc = 0; nc < array.GetLength(0); nc++)
                {
                    arow = arow + array[nc, nr].ToString("F2") + " ";
                }
                arow = arow.Trim() + "\r\n";
                rows = rows + arow;
                if (nr % 2 == 0) //이부분 더 크게 하면 오히려 늦어 진다..
                {
                    File.AppendAllText(fpn, rows);
                    rows = "";
                }
            }
            File.AppendAllText(fpn, rows);
        }

        
        public static bool MakeASCTextFileAsParallel(string fpn, string allHeader, double[,] array)

        {
            File.AppendAllText(fpn, allHeader);
            int rowYcount = array.GetLength(1);
            int colXcount = array.GetLength(0);
            var options = new ParallelOptions { MaxDegreeOfParallelism = Environment .ProcessorCount*2 };
            string[] rows = new string[rowYcount];
            StringBuilder[] sbs= new StringBuilder[rowYcount];
            Parallel.For(0, rowYcount, options, delegate (int ry)
            {
                sbs[ry] = new StringBuilder();
                for (int cx = 0; cx < colXcount; cx++)
                {
                    sbs[ry].Append(array[cx, ry].ToString("F2") + " ");
                }
                sbs[ry].Append("\r\n");
            });
            StringBuilder sbALL = new StringBuilder();
            for (int nr = 0; nr < rowYcount; nr++)
            {
                sbALL.Append(sbs[nr]);
            }
            File.AppendAllText(fpn, sbALL.ToString());
            return true;
        }


        public static bool MakeASCTextFileAsParallel_old(string fpn, string allHeader, double[,] array)
        {
            File.AppendAllText(fpn, allHeader);
            int rowYcount = array.GetLength(1);
            int colXcount = array.GetLength(0);
            var options = new ParallelOptions { MaxDegreeOfParallelism = Environment .ProcessorCount *4 };
            string[] rows = new string[rowYcount];
            Parallel.For(0, rowYcount, options, delegate (int ry)
            {
                string arow = "";
                for (int cx = 0; cx < colXcount; cx++)
                {
                    arow = arow + array[cx, ry].ToString() + " ";
                }
                arow = arow.Trim() + "\r\n";
                rows[ry] = arow;
            });
            for (int nr = 0; nr < rowYcount; nr++)
            {
                File.AppendAllText(fpn, rows[nr]);
            }
            return true;
        }

        public static SortedList<int, string> ReadVatFile(string sourceFPN)
        {
            SortedList<int, string> values = new SortedList<int, string>();
            StreamReader reader = new StreamReader(sourceFPN, System.Text.Encoding.Default);
            while (!reader.EndOfStream)
            {
                string line = reader.ReadLine();
                string[] parts = line.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);
                int attrValue = 0;
                if (parts != null && parts.Length == 2 && int.TryParse(parts[0], out attrValue)
                                && !values.ContainsKey(attrValue))
                {
                    values.Add(attrValue, parts[1]);
                }
                else
                {
                    Console.WriteLine(string.Format("Values in VAT file ({0}) are invalid, or attributes count are more than 1.{1}", sourceFPN, "\r\n"));
                    Console.WriteLine(string.Format("Each grid value must have one attribute[gridValue, attributeValue]."));
                }
            }
            reader.Close();
            return values;
        }

        public static SortedList<int, string[]> ReadVatFile(string sourceFPN, ValueSeparator separator = ValueSeparator.COMMA)
        {
            SortedList<int, string[]> values = new SortedList<int, string[]>();
            StreamReader reader = new StreamReader(sourceFPN, System.Text.Encoding.Default);
            while (!reader.EndOfStream)
            {
                string line = reader.ReadLine();
                string[] sep = GetTextFileValueSeparator(separator);
                string[] parts = line.Split( sep, StringSplitOptions.RemoveEmptyEntries);
                if (parts != null && parts.Length > 1)
                {
                    int keyvalue = 0;
                    if (int.TryParse(parts[0], out keyvalue) && !values.ContainsKey(keyvalue))
                    {
                        string[] attValues = new string[parts.Length - 1];
                        for (int ne = 0; ne <= parts.Length - 2; ne++)
                        {
                            attValues[ne] = parts[ne + 1];
                        }
                        values.Add(keyvalue, attValues);
                    }
                }
            }
            reader.Close();
            return values;
        }
        
        public static string[] GetTextFileValueSeparator(cTextFile.ValueSeparator valueSeparator)
        {
            string[] sepArray = null;
            switch (valueSeparator)
            {
                case cTextFile.ValueSeparator.COMMA:
                    sepArray = new string[] { "," };
                    break;
                case cTextFile.ValueSeparator.SPACE:
                    sepArray = new string[] { " " };
                    break;
                case cTextFile.ValueSeparator.FORWARD_SLASH:
                    sepArray = new string[] { "/" };
                    break;
                case cTextFile.ValueSeparator.TAB:
                    sepArray = new string[] { "\r\n" };
                    break;
                case cTextFile.ValueSeparator.ALL:
                    sepArray = new string[] { ",", " ", "\r\n" };
                    break;
                default:
                    sepArray = new string[] { ",", " ", "\r\n" };
                    break;
            }
            return sepArray;
        }


        public static void MakeTextFileUisngTextInTextFile(string strSourceFPN, string strTagetFPN,
            int startingLineIndex , int endingLineIndex , int[] colidxs, ValueSeparator sepSource, 
            bool onlyNumeric, ValueSeparator  sepResult)
        {
            if (File.Exists(strTagetFPN) == true) { File.Delete(strTagetFPN); }
            string[] sepsSource = GetTextFileValueSeparator(sepSource);
            string[] sepsResult = GetTextFileValueSeparator(sepResult);
            string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
            StringBuilder sb = new StringBuilder();
            if(startingLineIndex ==-1 )
            {
                startingLineIndex = 0;
            }
                if(endingLineIndex ==-1)
            {
                endingLineIndex = Lines.Length;
            }
            for (int n = 0; n < Lines.Length ; n++)
            {
                if  (n>=startingLineIndex && n<= endingLineIndex )
                {
                    string[] texts = Lines[n].Split(sepsSource, StringSplitOptions.RemoveEmptyEntries);
                    string v="" ;
                    string vLine = "";
                    for (int id = 0; id < colidxs.Length; id++)
                    {
                        if (texts.Length >= colidxs[id])
                        {
                            if (onlyNumeric == true)
                            {
                                if (cComTools.IsNumeric(texts[colidxs[id]]) == true) { v = texts[colidxs[id]].Trim(); }
                            }
                            else
                            {
                                v = texts[colidxs[id]].Trim();
                            }
                        }
                        if (v.Length>0 && v.Substring(v.Length - 1, 1) == sepsResult[0])
                        {
                            v = v.Substring(0, v.Length - 1);
                        }

                        if (vLine.Trim() == "")
                        {
                            vLine = v;
                        }
                        else
                        {
                            vLine = vLine + sepsResult[0] + v;
                        }
                    }
                    if (vLine != "")
                    {
                        sb.Append(vLine + "\r\n");
                    }
                }
                if (n==endingLineIndex )
                {
                    break;
                }
            }
            File.AppendAllText(strTagetFPN, sb.ToString());
        }

        public static void MakeTextFileUisngTextInTextFile(string strSourceFPN, string strTagetFPN, 
          string startingText, string endingText, int[] colidxs, ValueSeparator sepSource,
          bool onlyNumeric, ValueSeparator sepResult)
        {
            if (File.Exists(strTagetFPN) == true) { File.Delete(strTagetFPN); }
            string[] seps = GetTextFileValueSeparator(sepSource);
            string[] sepsResult = GetTextFileValueSeparator(sepResult);
            string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
            StringBuilder sb = new StringBuilder();
            bool started = false;
            bool endingConditionApplied = true;
            if (startingText == "") { started = true; }
            if (endingText == "") { endingConditionApplied = false; }
            for (int n = 0; n < Lines.Length; n++)
            {
                if (started==false && Lines[n].Contains(startingText))
                {
                    started = true;
                }
                if (started ==true)
                {
                    string[] texts = Lines[n].Split(seps, StringSplitOptions.RemoveEmptyEntries);
                    string v="";
                    string vLine = "";
                    for (int id = 0; id < colidxs.Length; id++)
                    {
                        if (texts.Length > id)
                        {
                            if (onlyNumeric == true)
                            {
                                if (cComTools.IsNumeric(texts[colidxs[id]]) == true) { v = texts[colidxs[id]].Trim(); }
                            }
                            else
                            {
                                v = texts[colidxs[id]].Trim();
                            }
                        }
                        if (v.Substring(v.Length - 1, 1) == sepsResult[0])
                        {
                            v = v.Substring(0, v.Length - 1);
                        }
                        if (vLine.Trim() == "")
                        {
                            vLine = v;
                        }
                        else
                        {
                            vLine = vLine + sepsResult[0] + v;
                        }
                    }                        
                    if (vLine != "")
                    {
                        sb.Append(vLine + "\r\n");
                    }
                }
                if (started==true && endingConditionApplied == true && Lines[n].Contains(endingText))
                {
                    break;
                }
            }
            File.AppendAllText(strTagetFPN, sb.ToString());
        }


        /// <summary>
        /// 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
        /// </summary>
        /// <param name="strSourceFPN"></param>
        /// <param name="strTagetFPN"></param>
        /// <param name="TagetLine"></param>
        /// <param name="strTextToReplace"></param>
        /// <remarks></remarks>
        public static void ReplaceALineInTextFile(string strSourceFPN, string strTagetFPN, int TagetLine, string strTextToReplace)
        {
            try
            {
                string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
                Lines[TagetLine - 1] = strTextToReplace;
                System.IO.File.WriteAllLines(strTagetFPN, Lines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
        /// </summary>
        /// <param name="strSourceFNP"></param>
        /// <param name="strTagetFNP"></param>
        /// <param name="ContainedTextInALine"></param>
        /// <param name="strTextToReplace"></param>
        /// <remarks></remarks>
        public static void ReplaceALineInTextFile(string strSourceFNP, string strTagetFNP, string ContainedTextInALine, string strTextToReplace)
        {
            try
            {
                string[] Lines = System.IO.File.ReadAllLines(strSourceFNP);
                for (int n = 0; n < Lines.Length; n++)
                {
                    if (Lines[n].Contains(ContainedTextInALine))
                    {
                        Lines[n] = strTextToReplace;
                    }
                }
                System.IO.File.WriteAllLines(strTagetFNP, Lines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static string[] ReplaceALineInStringArray(string[] sourceArray, string textToFindInALine, string textToReplace)
        {
            try
            {
                for (int n = 0; n < sourceArray.Length; n++)
                {
                    if (sourceArray[n].Contains(textToFindInALine))
                    {
                        sourceArray[n] = textToReplace;
                    }
                }
                return sourceArray;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static void ReplaceTextInTextFile(string strSourceFNP, string strTagetFNP, string strTextToFind, string strTextToReplace,
            int startingLineIndex = 0, int endingLineIndex = 0)
        {
            try
            {
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                int intTotCountLine = strLines.Length;
                int intNLine = 0;
                int nMax = 0;
                if (startingLineIndex > -1 && endingLineIndex > -1)
                {
                    nMax = endingLineIndex;
                }
                else
                {
                    nMax = intTotCountLine - 1;
                }
                if (startingLineIndex == -1) startingLineIndex = 0;
                for (intNLine = startingLineIndex; intNLine <= nMax; intNLine++)
                {
                    strLines[intNLine] = strLines[intNLine].Replace(strTextToFind, strTextToReplace);
                }
                System.IO.File.WriteAllLines(strTagetFNP, strLines);
                GC.Collect();
            }
            catch (Exception ex)
            {
                GC.Collect();
                throw ex;
            }
        }

        public static void ReplaceTextInTextFile(string strSourceFNP, string strTextToFind, string strTextToReplace,
             int startingLineIndex = 0, int endingLineIndex = 0)
        {
            try
            {
                String tmpFPN = "";
                tmpFPN =Path.Combine (Path.GetDirectoryName (strSourceFNP ), Path.GetFileNameWithoutExtension(strSourceFNP ) + ".tmp");
                if (File.Exists (tmpFPN)==true) { File.Delete(tmpFPN); }
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                foreach (string line in File.ReadLines(strSourceFNP))
                {
                   string newL =  line.Replace(strTextToFind, strTextToReplace);
                    System.IO.File.AppendAllText (tmpFPN , newL + "\r\n");
                }
                File.Delete(strSourceFNP);
                cComTools.timeDelay();
                File.Move(tmpFPN, strSourceFNP);
                GC.Collect();
            }
            catch (Exception ex)
            {
                GC.Collect();
                throw ex;
            }
        }

        public static void ReplaceTextInASCiiRasterRange(cAscRasterReader inAscRaster, string strTagetFNP,
            string strTextToFind, string strTextToReplace,
             int tlXcol, int tlYrow, int lrXcol, int lrYrow)
        {
            if (File.Exists (strTagetFNP )==true) { File.Delete(strTagetFNP); }
    
            File.AppendAllText(strTagetFNP, inAscRaster.HeaderStringAll);
            double vToF = 0;
            double.TryParse(strTextToFind, out vToF);
            for (int r = 0; r < inAscRaster.Header.numberRows; r++)
            {
                if (r >= tlYrow && r <= lrYrow)
                {
                    StringBuilder sbInAline = new StringBuilder();
                    for (int c = 0; c < inAscRaster.Header .numberCols; c++)
                    {
                        double av = inAscRaster.ValueFromTL(c, r);
                        string strv = av.ToString();
                        if (c >= tlXcol && c <= lrXcol)
                        {
                            if(av == vToF)
                            {
                                strv = strTextToReplace.Trim ();
                            }
                        }
                        sbInAline.Append(strv + " ");
                    }
                    File.AppendAllText(strTagetFNP, sbInAline.ToString() + "\r\n");
                }
                else
                {
                    File.AppendAllText(strTagetFNP, inAscRaster.OneRowContainsValuesFromTop(r) + "\r\n");
                }
            }
        }

        public static void ReplaceLineByLineInTextFile(string strSourceFNP, string strTagetFNP, string strTextToFind, string strTextToReplace,
            int startingLineIndex = 0, int endingLineIndex = 0)
        {
            try
            {
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                int intTotCountLine = strLines.Length;
                int intNLine = 0;
                int nMax = 0;
                if (startingLineIndex > -1 && endingLineIndex > -1)
                {
                    nMax = endingLineIndex;
                }
                else
                {
                    nMax = intTotCountLine - 1;
                }
                for (intNLine = startingLineIndex; intNLine <= nMax ; intNLine++)
                {
                        if (strLines[intNLine].Trim () == strTextToFind .Trim ())
                        {
                            strLines[intNLine] = strTextToReplace.Trim();
                        }
                }
                System.IO.File.WriteAllLines(strTagetFNP, strLines);
                GC.Collect();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static void RemoveLinesFromTextFile(string strSourceFNP, string strTagetFNP, int lineIndexToStart = 0, int lineIdexToEnd=0)
        {
            try
            {
                string[] lines = System.IO.File.ReadAllLines(strSourceFNP);
               List<string> lst = lines.ToList();
                for (int r=lineIndexToStart; r<= lineIdexToEnd;r++)
                {
                    lst.RemoveAt(lineIndexToStart); // 인덱스 개수가 계속 줄어드니까.. 이렇게 해야한다.
                }
                string[] newLines = lst.ToArray();
                System.IO.File.WriteAllLines(strTagetFNP, newLines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static void InsertAlineIntoTextFile(string strSourceFNP, string strTagetFNP, string txtToInsert,
    int lineIndexToInsert = 0)
        {
            try
            {
                string[] lines = System.IO.File.ReadAllLines(strSourceFNP);
                string[] newLines = new string[lines.Length+1];
                bool inserted = false;
                for (int y = 0; y<lines.Length+1;y++)
                {
                    if (y==lineIndexToInsert )
                    {
                        newLines[y] = txtToInsert;
                        inserted = true;
                    }
                    else
                    {
                        if (inserted ==true )
                        {
                            newLines[y] = lines[y-1];
                        }
                        else
                        {
                            newLines[y] = lines[y];
                        }
                    }
                }
                System.IO.File.WriteAllLines(strTagetFNP, newLines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static DataTable ReadTextFileAndSetDataTable(string FPNsource, cTextFile.ValueSeparator valueSeparator, int fieldCount = 0)
        {
            try
            {
                if (string.IsNullOrEmpty(FPNsource) || File.Exists(FPNsource) == false)
                {
                    Console.WriteLine("Source text file and data is invalid.   ");
                    return null;
                }
                StreamReader reader = new StreamReader(FPNsource, System.Text.Encoding.Default);
                DataTable dt = new DataTable();
                dt.BeginLoadData();
                int intL = 0;
                while (!reader.EndOfStream)
                {
                    string line = reader.ReadLine();
                    string[] sep = GetTextFileValueSeparator(valueSeparator);
                    string[] parts = line.Split(sep, StringSplitOptions.RemoveEmptyEntries);
                    if (string.IsNullOrEmpty(parts[0].ToString().Trim()))
                    {
                        Console.WriteLine(string.Format("{0} line has empty value. Exit reading text file process.", intL + 1));
                        break;
                    }
                    int nFieldCount = parts.Length;
                    if (intL == 0)
                    {
                        if (fieldCount > 0 && nFieldCount != fieldCount)
                        {
                            Console.WriteLine("Data series number in text file is different with user settings.   ");
                            return null;
                        }
                        foreach (string ele in parts)
                        {
                            dt.Columns.Add(ele.ToString());
                        }
                    }
                    else
                    {
                        DataRow nr = dt.NewRow();
                        for (int nG1 = 0; nG1 < nFieldCount; nG1++)
                        {
                            nr.ItemArray[nG1] = parts[nG1].ToString();
                        }
                        dt.Rows.Add(nr);
                    }
                    intL += 1;
                }
                dt.EndLoadData();
                return dt;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static string[] ReadGRMoutFileAndMakeStringArray(string[] BaseString, string FPNsource, 
            int rowNtoBeginRead, int colNtoRead, string colName, string[] SeparatorInSourceFile, 
            string SeparatorInReturnArray, bool valueAsInteger)
        {
            if (string.IsNullOrEmpty(FPNsource) || File.Exists(FPNsource) == false)
            {
                Console.WriteLine("Source text file and data is invalid.   ");
                return null;
            }
            string[] Lines = System.IO.File.ReadAllLines(FPNsource);
            int nr = 0;
            if (BaseString[0] == null || string.IsNullOrEmpty(BaseString[0].ToString().Trim()))
            {
                BaseString[nr] = colName;
            }
            else
            {
                BaseString[0] = BaseString[0] + SeparatorInReturnArray + colName;
            }
            for (int nl = 0; nl < Lines.Length; nl++)
            {
                if (nl >= rowNtoBeginRead - 1)
                {
                    nr += 1;
                    if ((nr + 1) > BaseString.Length)
                    {
                        Console.WriteLine("Input file contains more row count than base string array.   ");
                        break; 
                    }
                    string aLine = Lines[nl];
                    string[] parts = aLine.Split(SeparatorInSourceFile, StringSplitOptions.RemoveEmptyEntries);
                    string value = null;
                    StringBuilder stbAline = new StringBuilder();
                    if (valueAsInteger == true)
                    {
                        value = Convert.ToInt32(parts[colNtoRead - 1]).ToString().Trim();
                    }
                    else
                    {
                        value = parts[colNtoRead - 1].ToString().Trim();
                    }
                    if (BaseString[nr] == null || string.IsNullOrEmpty(BaseString[nr].ToString().Trim()))
                    {
                        BaseString[nr] = value;
                    }
                    else
                    {
                        BaseString[nr] = BaseString[nr] + SeparatorInReturnArray + value;
                    }
                }
            }
            return BaseString;
        }

        public static int GetValueStartingRowNumber(string fpnSource, cTextFile.ValueSeparator valueSeparator, string timeFieldName = "")
        {
            string[] Lines = System.IO.File.ReadAllLines(fpnSource);
            int nr = 0;
            string[] sepArray = GetTextFileValueSeparator(valueSeparator);
            for (int nl = 0; nl < Lines.Length; nl++)
            {
                string aLine = Lines[nl];
                string[] parts = aLine.Split(sepArray, StringSplitOptions.RemoveEmptyEntries);
                if (parts.Length > 1)
                {
                    if (!string.IsNullOrEmpty(timeFieldName) && parts[0] == timeFieldName)
                    {
                        nr = nl + 2;
                        break; 
                    }
                }
            }
            return nr;
        }
    }
}
