/*
Copyright 2018 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows.Controls;

namespace DMValidator
{
    enum LogLevel
    {
        Verbose,
        Information,
        Warning,
        Error,
        Critical
    }

    interface ILogger
    {
        void Log(LogLevel logLevel, string message);
    }

    class Logger : ILogger
    {
        public ListView TargetListView { get; set; }
        public string TargetLogPath
        {
            get
            {
                return _targetLogPath;
            }

            set
            {
                if (_targetLogPath == value)
                {
                    return;
                }

                _targetLogPath = value;
                CreateNewFile();
            }
        }

        public void CreateNewFile()
        {
            _targetLogFile = _targetLogPath + "\\" + GetLogFileName();
            using (StreamWriter sw = File.CreateText(_targetLogFile))
            {
            }
        }

        public Logger()
        {
        }

        private static string GetLogFileName()
        {
            return "DMValidator_" + DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss");
        }

        public void Log(LogLevel logLevel, string message)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(DateTime.Now.ToString("HH:mm:ss"));
            sb.Append(": " + logLevel.ToString().PadRight(12) + ": ");
            sb.Append(message);

            string formattedMessage = sb.ToString();

            Debug.WriteLine(formattedMessage);

            if (TargetListView != null && logLevel >= LogLevel.Information)
            {
                TargetListView.Items.Add(formattedMessage);
            }

            if (_targetLogPath != null)
            {
                using (StreamWriter sw = File.AppendText(_targetLogFile))
                {
                    sw.WriteLine(formattedMessage);
                }
            }
        }

        string _targetLogPath;
        string _targetLogFile;
    }
}