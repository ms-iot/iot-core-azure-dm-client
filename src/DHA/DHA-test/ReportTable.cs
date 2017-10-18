/*
Copyright 2017 Microsoft
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

using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DHA_test
{
    internal class ReportTable
    {
        public static ReportTable Instance = new ReportTable();

        private ReportTable()
        {
            var connectionString = AppSettings.Current.GetValue("dha-storage");
            var cloudStorage = CloudStorageAccount.Parse(connectionString);
            var tableClient = cloudStorage.CreateCloudTableClient();
            _table = tableClient.GetTableReference("dhaReportTable");
        }

        public IEnumerable<DynamicTableEntity> Query(DateTimeOffset start, DateTimeOffset end, string deviceId, string correlationId)
        {
            var rowKeyStartPattern = deviceId + "-";
            var rowKeyEndPattern = deviceId + "."; // '.' is the char after '-' in ascii

            var query = _table.CreateQuery<DynamicTableEntity>()
                .Where(d => d.PartitionKey == "" &&
                       d.RowKey.CompareTo(rowKeyStartPattern) >= 0 && d.RowKey.CompareTo(rowKeyEndPattern) <= 0 &&
                       d.Timestamp >= start && d.Timestamp <= end &&
                       d.Properties["CorrelationId"].StringValue == correlationId
                       );
            return query.ToList();
        }

        private CloudTable _table;
    }
}
