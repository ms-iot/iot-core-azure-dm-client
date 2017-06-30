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

#r "Microsoft.WindowsAzure.Storage"

using System;
using Microsoft.WindowsAzure.Storage.Table;

public class NonceEntity : TableEntity
{
    // RowKey = Device ID
    public string Nonce { get; set; }
}

public class NonceCloudTable
{
    public NonceCloudTable(CloudTable nonceTable, TraceWriter log)
    {
        _nonceTable = nonceTable;
        _log = log;
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
        await _nonceTable.ExecuteAsync(operation);
    }

    public async Task<string> GetNonceForDeviceAsync(string deviceId)
    {
        var op = TableOperation.Retrieve<NonceEntity>(GetPartitionKeyForDeviceId(deviceId), deviceId);
        var opResult = await _nonceTable.ExecuteAsync(op);
        if (opResult.Result == null)
        {
            var msg = $"Cannot find nonce for {deviceId}";
            _log.Error(msg);
            throw new InvalidOperationException(msg);
        }
        var nonce = ((NonceEntity)opResult.Result).Nonce;
        _log.Info($"DeviceId: {deviceId} Nonce: {nonce}");
        return nonce;
    }

    public async Task DeleteNonceForDeviceAsync(string deviceId)
    {
        var entity = new DynamicTableEntity(GetPartitionKeyForDeviceId(deviceId), deviceId);
        entity.ETag = "*";
        var deleteOp = TableOperation.Delete(entity);
        await _nonceTable.ExecuteAsync(deleteOp);
    }

    private TraceWriter _log;
    private CloudTable _nonceTable;
}
