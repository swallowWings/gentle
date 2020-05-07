using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
    public class cCalculator
    {
        public static bool checkIsAlgebraicOperator(string inString)
        {
           switch (inString.Trim())
            {
                case "+":
                    return true;
                case "-":
                    return true;
                case "*":
                    return true;
                case "/":
                    return true;
                default:
                    return false;
            }
        }

        public static string[] SplitElementsInAlgebraicEquation(string inputString, string[] sep)
        {

            inputString = inputString.Trim();
            string firstChar = inputString.Substring(0, 1).Trim();
            string[] values = inputString.Split(sep, StringSplitOptions.RemoveEmptyEntries);
            if (firstChar == "-")
            {
                values[0] = "-" + values[0];
            }
            for (int e=0;e<values.Length;e++)
            {
                values[e] = values[e].Trim();
            }
            return values;
        }


        public static string getOperatorFromString(string inString)
        {
            //string[] operators = { ",", "(", ")", "=+", "=-", ">=+", ">=-", "<=+", "<=-", ">+", ">-", "<+", "<-", "/+", "/-", "^+", "^-", ">=", "<=", "=", ">", "<", "+", "-", "*", "/", "^" };
            string[] operators_sep = { ",", "(", ")"};
            foreach (string op in operators_sep)
            {
                if (inString.Contains(op.Trim()))
                {
                    return op;
                }
            }

            string[] operators_condition = {">=", "<=", "=", ">", "<"};
            foreach (string op in operators_condition)
            {
                if (inString.Contains(op.Trim()))
                {
                    return op;
                }
            }
            string[] operators_Math = { "*", "/", "^", "+", "-" };
            foreach (string op in operators_Math)
            {
                if (inString.Contains (op.Trim()))
                {
                    return op;
                }
            }
            //위에서 다 걸러 지지만.. 혹시 걸러 지지 않으면 여기서 처리
            string[] operators_con_ToRev = { "=+", "=-", ">=+", ">=-", "<=+", "<=-", ">+", ">-", "<+", "<-", "/+", "/-", "^+", "^-" };
            foreach (string op in operators_con_ToRev)
            {
                if (inString.Contains(op.Trim()))
                {
                    string remains = op.Substring(0, op.Length - 1);
                    return remains;
                }
            }

            //위에서 다 걸러 지지만.. 혹시 걸러 지지 않으면 여기서 처리
            string[] operators_con_ToRev2 = { "= +", "= -", ">= +", ">= -", "<= +", "<= -", "> +", "> -", "< +", "< -", "/ +", "/ -", "^ +", "^ -" };
            foreach (string op in operators_con_ToRev2)
            {
                if (inString.Contains(op.Trim()))
                {
                    string remains = op.Substring(0, op.Length - 2);
                    return remains;
                }
            }

            return "";
        }

        public static double[,] calculate2DArryUsingMathFunction(double[,] asc2D,
            cData.MathFunctionType fType, double expV=1, double nodataValue = -9999)
        {
            int ny = asc2D.GetLength(1);
            int nx = asc2D.GetLength(0);
            double[,] resultArr = null;
            resultArr = new double[nx, ny];  
            ParallelOptions options = new ParallelOptions();
            options.MaxDegreeOfParallelism = Environment.ProcessorCount;
            switch (fType)
            {
                case cData.MathFunctionType.Pow:
                    Parallel.For(0, ny, options, delegate (int y)
                    {
                        for (int x = 0; x < nx; x++)
                        {
                            double v = asc2D[x, y];

                            if (v == nodataValue)
                            {
                                resultArr[x, y] = nodataValue;
                            }
                            else if (v == 0)
                            {
                                resultArr[x, y] = 0;
                            }
                            else if (v < 0)
                            {
                                string em = string.Format("A value at [{0}, {1}] is less than zero. It cannot be applied. ", x, y);
                                Console.WriteLine(em);
                                return;
                            }
                            else 
                            {
                                resultArr[x, y] = Math.Pow(v, expV);
                            }
                        }
                    });
                    break;
                case cData.MathFunctionType.Abs:
                    Parallel.For(0, ny, options, delegate (int y)
                    {
                        for (int x = 0; x < nx; x++)
                        {
                            double v = asc2D[x, y];
                            if (v == nodataValue)
                            {
                                resultArr[x, y] = nodataValue;
                            }
                            else if (v == 0)
                            {
                                resultArr[x, y] = 0;
                            }
                            else 
                            {
                                resultArr[x, y] =  Math.Abs(v);
                            }

                        }
                    });
                    break;
            }
            return resultArr;
        }


        public static double[,] calculate2DArryUsing2TermAlgebra(string inOperator, bool is1ASC, bool is2ASC,
            bool is1ASCnodataAsZero, bool is2ASCnodataAsZero,
            double[,] asc1 = null, double[,] asc2 = null, double value1 = 0, double value2 = 0, double nodataValue = -9999)
        {
            double[,] resultArr = null;
            if (is1ASC == true)
            { resultArr = new double[asc1.GetLength(0), asc1.GetLength(1)]; }
            else
            { resultArr = new double[asc2.GetLength(0), asc2.GetLength(1)]; }

            int ny = resultArr.GetLength(1);
            int nx = resultArr.GetLength(0);
            ParallelOptions options = new ParallelOptions();
            options.MaxDegreeOfParallelism = Environment.ProcessorCount;
            //for (int y  = 0; y< ny; y++)
            //{
            Parallel.For(0, ny, options, delegate (int y)
            {
                for (int x = 0; x < nx; x++)
                {
                    double v1;
                    double v2;
                    if (is1ASC == true)
                    {
                        v1 = asc1[x, y];
                        if (is1ASCnodataAsZero == true && v1 == nodataValue) { v1 = 0; }
                    }
                    else
                    { v1 = value1; }

                    if (is2ASC == true)
                    {
                        v2 = asc2[x, y];
                        if (is2ASCnodataAsZero == true && v2 == nodataValue) { v2 = 0; }
                    }
                    else
                    { v2 = value2; }

                    bool goCal = true;
                    if (is1ASC == true && is1ASCnodataAsZero == false && v1 == nodataValue)
                    { goCal = false; }
                    if (is2ASC == true && is2ASCnodataAsZero == false && v2 == nodataValue)
                    { goCal = false; }

                    if (goCal == true)
                    {
                        resultArr[x, y] = algebraicCal(inOperator, v1, v2, nodataValue);
                    }
                    else
                    {
                        resultArr[x, y] = nodataValue;
                    }
                }
            });
        //}
            return resultArr;
        }

        public static double[,] calculate2DArryUsingCondition(string ConOperator, bool is1ASC, bool is2ASC, bool isTasc, bool isFasc,
            double[,] asc1 = null, double[,] asc2 = null, double[,] ascT = null, double[,] ascF = null,
            double value1 = 0, double value2 = 0, double valueT = 0, double valueF = 0,
            double nodataValue = -9999)
        {
            double[,] resultArr = null;
            if (is1ASC == true)
            { resultArr = new double[asc1.GetLength(0), asc1.GetLength(1)]; }
            else
            { resultArr = new double[asc2.GetLength(0), asc2.GetLength(1)]; }

            int ny = resultArr.GetLength(1);
            int nx = resultArr.GetLength(0);
            ParallelOptions options = new ParallelOptions();
            options.MaxDegreeOfParallelism = Environment.ProcessorCount;
            //for(int y = 0;y<ny;y++)
            //{

            Parallel.For(0, ny, options, delegate (int y)
            {
                for (int x = 0; x < nx; x++)
                {
                    //if (x==192 && y==55)
                    //{
                    //    int a = 1;
                    //}
                    double v1;
                    double v2;
                    double vT;
                    double vF;
                    if (is1ASC == true)
                    { v1 = asc1[x, y]; }
                    else
                    { v1 = value1; }

                    if (is2ASC == true)
                    { v2 = asc2[x, y]; }
                    else
                    { v2 = value2; }

                    if (isTasc == true)
                    { vT = ascT[x, y]; }
                    else
                    { vT = valueT; }

                    if (isFasc == true)
                    { vF = ascF[x, y]; }
                    else
                    { vF = valueF; }

                    resultArr[x, y] = cCalculator.conditionalCal(ConOperator, v1, v2, vT, vF, nodataValue);
                }
            });
            //}
            return resultArr;
        }


        public static double conditionalCal(string conditionString, double conValue1, double conValue2, double TrueValue, double FalseValue, double nodataValue)
        {
            double vout = 0;
            switch (conditionString)
            {
                case ">":
                    if (conValue1 > conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "<":
                    if (conValue1 < conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=":
                    if (conValue1 == conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case ">=":
                    if (conValue1 >= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=>":
                    if (conValue1 >= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "<=":
                    if (conValue1 <= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=<":
                    if (conValue1 <= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                default:
                    vout = nodataValue;
                    break;
            }
            return vout;
        }

        public static double algebraicCal(string conditionString, double v1, double v2,  double nodataValue)
        {
            double vout = 0;
            switch (conditionString)
            {
                case "+":
                    vout = v1 + v2;
                    break;
                case "-":
                    vout = v1 - v2;
                    break;
                case "*":
                    vout = v1 * v2;
                    break;
                case "/":
                    if (v2 != 0)
                    { vout = v1 / v2; }
                    else
                    { vout = nodataValue; }
                    break;
                case "^":
                    vout = Math.Pow(v1, v2);
                    break;
                default:
                    vout = nodataValue;
                    break;
            }
            return vout;
        }
    }
}
