using System;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Popups;
using Windows.Devices.Geolocation;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls.Primitives;

namespace IoTDMBackground
{
    class WaveGenerator
    {
        public double Min;
        public double Max;
        public int Period; // seconds
        public double Deviation; // bigger for more randomness

        private int i = 0;
        private Random rnd = new Random();

        public double GetNext()
        {
            double div = (i % Period);
            double mult = div / Period;

            i++;

            var current = Min + Math.Sin(Math.PI * mult) * (Max - Min) + ((rnd.NextDouble() - 0.5) * Deviation);

            return current;
        }
    }
    class StepGenerator
    {
        public class StepDuration
        {
            public double Value;
            public int Duration; // seconds
        }

        public StepDuration[] Steps;

        public double Deviation; // bigger for more randomness

        private int i = 0;
        private int step = 0;
        private Random rnd = new Random();
        private double currentDeviation = 0;

        public double GetNext()
        {
            var current = Steps[step].Value + currentDeviation;

            i++;

            var currentStep = Steps[step];
            if (i >= currentStep.Duration)
            {
                i = 0;
                step++;
                currentDeviation = (rnd.NextDouble() - 0.5) * Deviation;
                if (step == Steps.Length)
                {
                    step = 0;
                }
            }

            return current;
        }
    }
}