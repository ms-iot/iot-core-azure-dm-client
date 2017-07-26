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

using System;
using System.Threading.Tasks;
using Microsoft.WindowsAzure.Storage.Table;
using Microsoft.WindowsAzure.Storage;

namespace DHA_test
{
    public class NonceEntity : TableEntity
    {
        // RowKey = Device ID
        public string Nonce { get; set; }
    }

    public class NonceCloudTable
    {
        public static NonceCloudTable Instance = new NonceCloudTable();

        private NonceCloudTable()
        {
            var connectionString = AppSettings.Current.GetValue("dha-storage");
            var cloudStorage = CloudStorageAccount.Parse(connectionString);
            var tableClient = cloudStorage.CreateCloudTableClient();
            _table = tableClient.GetTableReference("nonceTable");
        }

        public async Task SetNonceForDeviceAsync(string deviceId, string nonce)
        {
            var entity = new NonceEntity()
            {
                PartitionKey = GetPartitionKeyForDeviceId(deviceId),
                RowKey = deviceId,
                Nonce = nonce
            };

            var operation = TableOperation.InsertOrReplace(entity);
            await _table.ExecuteAsync(operation);
        }

        public async Task<string> GetNonceForDeviceAsync(string deviceId)
        {
            var op = TableOperation.Retrieve<NonceEntity>(GetPartitionKeyForDeviceId(deviceId), deviceId);
            var opResult = await _table.ExecuteAsync(op);
            if (opResult.Result == null)
            {
                var msg = $"Cannot find nonce for {deviceId}";
                throw new Exception(msg);
            }
            var nonce = ((NonceEntity)opResult.Result).Nonce;
            return nonce;
        }

        public async Task DeleteNonceForDeviceAsync(string deviceId)
        {
            var entity = new DynamicTableEntity(GetPartitionKeyForDeviceId(deviceId), deviceId);
            entity.ETag = "*";
            var deleteOp = TableOperation.Delete(entity);
            await _table.ExecuteAsync(deleteOp);
        }

        private string GetPartitionKeyForDeviceId(string deviceId)
        {
            return string.Empty;
        }

        private CloudTable _table;
    }

}
