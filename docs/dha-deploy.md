# Deploy Azure Resources for Device Health Attestation

Additional Azure resources are required to support Device Health Attestation.  Please see [Device Health Attestation Architecture](dha-architecture.md) for more details.  If Device Health Attestation is not required, this step can be skipped.

#### Make Sure Library Is Ready
- Follow the steps described in [Building the Device Management Binaries](building-the-dm-binaries.md).

#### Prepare Powershell with Azure Resource Management
Follow the steps described in [Install and configure Azure PowerShell](https://docs.microsoft.com/en-us/powershell/azure/install-azurerm-ps?view=azurermps-4.1.0)

Afterwards, execute the following in powershell:

- ```Import-Module AzureRM```
- ```Login-AzureRmAccount``` and login using your Microsoft account associated with your Azure subscription
- ```Get-AzureRmSubscription``` to get a list of azure subscriptions associated with your account
- ```Set-AzureRmContext -SubscriptionName "<your subscription name>" ``` to select the subscription where the resource will be deploy


#### Setup Device Health Attestation resources

Execute the following in powershell to deploy necessary for Device Health Attestation.

```
cd "<repo directory>\src\DHA\Deployment Scripts"
.\DHA-setup.ps1 -ResourceGroupName "<Your resource group name>" `
                -IotHubName "<Your existing IoTHub name>" `
                -ServiceBusName "<Service bus name to be created>" `
                -StorageName "<Service account name to be created>" `
                -AzureFunctionName "<Azure function name to be created>"
```
                   
Note: Due to Azure naming restrictions.  The parameter ```StorageName``` and ```AzureFunctionName``` must be only contain lower case alphanumeric characters.

Example:
```
.\DHA-setup.ps1 -ResourceGroupName "test-deploy-rg" `
                -IotHubName "test-deploy-iothub" `
                -ServiceBusName "test-deploy-servicebus" `
                -StorageName "testdeploystorage" `
                -AzureFunctionName "dhafunction"
```

If you wish to adjust the SKU or pricing tier of the Azure resources, you can modify the corresponding [Azure Resources Manger](https://docs.microsoft.com/en-us/azure/azure-resource-manager/resource-group-authoring-templates) templates under [deployment scripts](../src/DHA/Deployment%20Scripts/).

For example, if you want to change account type of the Azure Storage account where the DHA health reports are stored, you can modify line 13 of [AzureStorageDeploy.json](../src/DHA/Deployment%20Scripts/AzureStorageDeploy.json).

```
    "storageAccountType": {
      "type": "string",
      "defaultValue": "<desired account type>",    <-- line 13
      "allowedValues": [
        "Standard_LRS",
        "Standard_GRS",
        "Standard_ZRS",
        "Premium_LRS"
      ],
      "metadata": {
        "description": "Storage Account type"
      }
    }
```

----

[Home Page](../README.md) | [Library Reference](library-reference.md)