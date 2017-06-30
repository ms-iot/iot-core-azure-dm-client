<#
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
#>

<#
.SYNOPSIS

This script deploy the necessary Azure resources for device health attestation.  The following resources will be created in the specified resource group:
    - an Azure Storage Account named "$StorageName" 
    - an Azure Service Bus namespace named "$ServiceBusName" 
    - an Azure Service Bus Queue named "DHA" in the service bus namespace above
    - an Azure Function named "$AzureFunctionName"
    - an Azure App Service Plan named "$AzureFunctionName"
    - an Azure Storage Account named "$AzureFunctionName" concatenated with "storage" (for example: "dhafunctionstorage")

This script will also link these resources together by creating:
    - a new Endpoint in the specified IoTHub pointing to the newly create service bus queue
    - a routing rule in the specified IoTHub to redirect DHA messages to the newly created endpoint above 

Note: Due to Azure naming restrictions.  The parameter "$StorageName" and "$AzureFunctionName" must be only contain lower case alphanumeric characters.

.DESCRIPTION

This script deploys the necessary Azure resources for device health attestation. 

.PARAMETER ResourceGroupName
String. The name of the resource group.

.PARAMETER IotHubName
String. The name of the Azure IotHub.

.PARAMETER ServiceBusName
String. The name of the Azure ServiceBus.

.PARAMETER StorageName
String. The name of the Azure Storage account.
Note: this must be all lower case alphanumeric to conform to Azure naming restrictions.

.PARAMETER AzureFunctionName
String. The name of the Azure Function.
Note: this must be all lower case alphanumeric to conform to Azure naming restrictions.

#>

Param(
    [Parameter(Mandatory=$true)] [string] $ResourceGroupName, 
    [Parameter(Mandatory=$true)] [string] $IotHubName, 
    [Parameter(Mandatory=$true)] [string] $ServiceBusName, 
    [Parameter(Mandatory=$true)] [string] $StorageName, 
    [Parameter(Mandatory=$true)] [string] $AzureFunctionName)

$ErrorActionPreference = 'stop'

import-module AzureRM

class ServiceBus
{
    [string] $Name
    [string] $ResourceGroupName

    ServiceBus([string] $Name, [string] $ResourceGroupName)
    {
        $this.Name = $Name
        $this.ResourceGroupName = $ResourceGroupName
    }

    CreateIfNotExist()
    {
        $serviceBus = Get-AzureRmServiceBusNamespace -ResourceGroup $this.ResourceGroupName -NamespaceName $this.Name -ErrorAction SilentlyContinue
        if (-not $serviceBus)
        {
            Write-Host "Creating new ServiceBus Namespace: $($this.Name)"
            $ResourceGroup = Get-AzureRmResourceGroup -Name $this.ResourceGroupName
            New-AzureRmResourceGroupDeployment -serviceBusNamespaceName $this.Name -ResourceGroupName $this.ResourceGroupName -TemplateFile .\AzureServiceBusDeploy.json 
            $serviceBus = Get-AzureRmServiceBusNamespace -ResourceGroup $this.ResourceGroupName -NamespaceName $this.Name
        }
    }
    
    [PSObject] CreateAuthorizationRuleIfNotExist([string] $Name, $Rights)
    {
        $rule = Get-AzureRmServiceBusNamespaceAuthorizationRule -ResourceGroup $this.ResourceGroupName -NamespaceName $this.Name -AuthorizationRuleName $Name -ErrorAction SilentlyContinue
        if (-not $rule)
        {
            Write-Host "Creating new authorization rule: $Name"
            $rule = New-AzureRmServiceBusNamespaceAuthorizationRule -ResourceGroup $this.ResourceGroupName -NamespaceName  $this.Name -AuthorizationRuleName $Name -Rights $Rights
        }
        return $rule
    }

    [PSObject] GetSendOnlyKey()
    {
        $ruleName = "SendOnly"
        $rights = @("Send")
        $rule = $this.CreateAuthorizationRuleIfNotExist($ruleName, $rights)
        return Get-AzureRmServiceBusNamespaceKey -ResourceGroup $this.ResourceGroupName -NamespaceName $this.Name -AuthorizationRuleName $RuleName
    }

    [PSObject] GetListenOnlyKey()
    {
        $ruleName = "ListenOnly"
        $rights = @("Listen")
        $rule = $this.CreateAuthorizationRuleIfNotExist($ruleName, $rights)
        return Get-AzureRmServiceBusNamespaceKey -ResourceGroup $this.ResourceGroupName -NamespaceName $this.Name -AuthorizationRuleName $ruleName
    }
}

class ServiceBusQueue
{
    [string] $Name
    [string] $ServiceBusName
    [string] $ResourceGroupName

    ServiceBusQueue([string] $Name, [string] $ServiceBusName, [string] $ResourceGroupName)
    {
        $this.Name = $Name
        $this.ServiceBusName = $ServiceBusName
        $this.ResourceGroupName = $ResourceGroupName
    }

    CreateIfNotExist()
    {
        $queue = Get-AzureRmServiceBusQueue -ResourceGroup $this.ResourceGroupName -NamespaceName $this.ServiceBusName -QueueName $this.Name -ErrorAction SilentlyContinue 
        if (-not $queue)
        {
            Write-Host "Creating new ServiceBus Queue: $($this.Name)"
            $ResourceGroup = Get-AzureRmResourceGroup -Name $this.ResourceGroupName
            New-AzureRmResourceGroupDeployment -serviceBusNamespaceName $this.ServiceBusName -serviceBusQueueName $this.Name -ResourceGroupName $this.ResourceGroupName -TemplateFile .\AzureServiceBusQueueDeploy.json
            $queue = Get-AzureRmServiceBusQueue -ResourceGroup $this.ResourceGroupName -NamespaceName $this.ServiceBusName -QueueName $this.Name
        }
    }

    [PSObject] CreateAuthorizationRuleIfNotExist([string] $Name, $Rights)
    {
        $rule = Get-AzureRmServiceBusQueueAuthorizationRule -ResourceGroup $this.ResourceGroupName -NamespaceName $this.ServiceBusName -QueueName $this.Name -AuthorizationRuleName $Name -ErrorAction SilentlyContinue
        if (-not $rule)
        {
            Write-Host "Creating new authorization rule: $Name"
            $rule = New-AzureRmServiceBusQueueAuthorizationRule -ResourceGroup $this.ResourceGroupName -NamespaceName  $this.ServiceBusName -QueueName $this.Name -AuthorizationRuleName $Name -Rights $Rights
        }
        return $rule
    }

    [PSObject] GetSendOnlyKey()
    {
        $ruleName = "SendOnly"
        $rights = @("Send")
        $rule = $this.CreateAuthorizationRuleIfNotExist($ruleName, $rights)
        return Get-AzureRmServiceBusQueueKey -ResourceGroup $this.ResourceGroupName -NamespaceName $this.ServiceBusName -QueueName $this.Name -AuthorizationRuleName $RuleName
    }

    [PSObject] GetListenOnlyKey()
    {
        $ruleName = "ListenOnly"
        $rights = @("Listen")
        $rule = $this.CreateAuthorizationRuleIfNotExist($ruleName, $rights)
        return Get-AzureRmServiceBusQueueKey -ResourceGroup $this.ResourceGroupName -NamespaceName $this.ServiceBusName -QueueName $this.Name -AuthorizationRuleName $ruleName
    }
}

class Storage
{
    [string] $Name
    [string] $ResourceGroupName

    Storage([string] $Name, [string] $ResourceGroupName)
    {
        $this.Name = $Name
        $this.ResourceGroupName = $ResourceGroupName
    }

    CreateIfNotExist()
    {
        $storage = Get-AzureRmStorageAccount  -ResourceGroup $this.ResourceGroupName -Name $this.Name -ErrorAction SilentlyContinue
        if (-not $storage)
        {
            Write-Host "Creating new azure storage: $($this.Name)"
            $ResourceGroup = Get-AzureRmResourceGroup -Name $this.ResourceGroupName
            New-AzureRmResourceGroupDeployment -storageAccountName $this.Name -ResourceGroupName $this.ResourceGroupName -TemplateFile .\AzureStorageDeploy.json
            $storage = Get-AzureRmStorageAccount -ResourceGroup $this.ResourceGroupName -Name $this.Name
        }
    }

    [string] GetConnectionString()
    {
        $key = Get-AzureRmStorageAccountKey -ResourceGroupName $this.ResourceGroupName -Name $this.Name
        return "DefaultEndpointsProtocol=https;AccountName=$($this.Name);AccountKey=$($key.Value[0])"
    }
}

class IoTHub
{
    [string] $Name
    [string] $ResourceGroupName

    IoTHub([string] $Name, [string] $ResourceGroupName)
    {
        $this.Name = $Name
        $this.ResourceGroupName = $ResourceGroupName
    }

    [PSObject] GetServiceConnectionString()
    {
        return Get-AzureRmIotHubConnectionString -ResourceGroupName $this.ResourceGroupName -Name $this.Name -KeyName "Service"
    }

    AddServiceBusQueueEndpoint($Name, $ConnectionString, $ResourceGroup, $SubscriptionId)
    {
        $iotHub = $this.GetInfo()
        $routingProperties = $iotHub.Properties.Routing

        # Check if the endpoint exist or not
        $endpoint = $routingProperties.Endpoints.ServiceBusQueues | where {$_.Name -eq $Name}
        if (-not $endpoint)
        {
            #Create new endpoint
            Write-host "Adding IoTHub Endpoint"

            $ServiceBusQueueEndpointProp = New-Object Microsoft.Azure.Management.IotHub.Models.PSRoutingServiceBusQueueEndpointProperties
            $ServiceBusQueueEndpointProp.Name = $Name
            $ServiceBusQueueEndpointProp.ConnectionString = $ConnectionString
            $ServiceBusQueueEndpointProp.ResourceGroup = $ResourceGroup
            $ServiceBusQueueEndpointProp.SubscriptionId = $SubscriptionId

            $routingProperties.Endpoints.ServiceBusQueues.Add($ServiceBusQueueEndpointProp)
            Set-AzureRmIotHub -Name $this.Name -ResourceGroupName $this.ResourceGroupName -RoutingProperties $routingProperties
        }
    }

    AddRoutingRule([string] $Name, [string] $Condition, [bool] $IsEnabled, [string]$EndpointName, $Source)
    {
        $iotHub = $this.GetInfo()
        $routes = $iotHub.Properties.Routing.Routes

        # Check if the endpoint exist or not
        $route = $routes | where {$_.Name -eq $Name}
        if (-not $route)
        {
            #Create new route
            Write-host "Adding IoTHub route for DHA"
            $route = New-Object Microsoft.Azure.Commands.Management.IotHub.Models.PSRouteMetadata
            $route.Name = $Name
            $route.Condition = $Condition
            $route.IsEnabled = $IsEnabled
            $route.EndpointNames = New-Object 'System.Collections.Generic.List[String]'            
            $route.EndpointNames.Add($EndpointName)
            $route.Source = $Source

            $routes.Add($route)
            Set-AzureRmIotHub -Name $this.Name -ResourceGroupName $this.ResourceGroupName -Routes $routes
        } 
    }

    [PsObject] GetInfo()
    {
        return Get-AzureRmIotHub -Name $this.Name -ResourceGroupName $this.ResourceGroupName
    }
}

class AzureFunction
{
    [string] $Name
    [string] $ResourceGroupName
    [string] $FunctionDirectory

    AzureFunction([string] $Name, [string] $ResourceGroupName)
    {
        $this.Name = $Name
        $this.ResourceGroupName = $ResourceGroupName
        $this.FunctionDirectory = "$PSScriptRoot\..\DHA\"
    }

    CreateIfNotExist([string] $DhaServiceBusListenConnectionString, [string] $DhaStorageConnectionString, [string] $IotHubServiceConnectionString)
    {
        $function = Get-AzureRmWebApp  -ResourceGroup $this.ResourceGroupName -Name $this.Name -ErrorAction SilentlyContinue
        if (-not $function)
        {
            Write-Host "Creating new azure function: $($this.Name)"
            New-AzureRmResourceGroupDeployment -appName $this.Name -ResourceGroupName $this.ResourceGroupName -TemplateFile .\azurefunctiondeploy.json `
                -dhaStorageConnectionString $DhaStorageConnectionString -dhaServiceBusConnectionString $DhaServiceBusListenConnectionString -iotHubServiceConnectionString $IotHubServiceConnectionString 
            $function = Get-AzureRmWebApp  -ResourceGroup $this.ResourceGroupName -Name $this.Name 
        }
    }

    CreateZipPackage($DestinationPath)
    {
        Push-Location $this.FunctionDirectory
        try
        {
            $files = @()
            $files += get-item -Path "DHAServiceBusQueueHandler"
            $files += get-item -Path "host.json"
            Compress-Archive -Path $files  -DestinationPath $DestinationPath -Force 
        }
        finally
        {
            Pop-Location
        }  
    }

    [PSobject] GetPublishingCredentials()
    {
        $resourceType = "Microsoft.Web/sites/config"
        $resourceName = "$($this.Name)/publishingcredentials"

        $publishingCredentials = Invoke-AzureRmResourceAction -ResourceGroupName $this.ResourceGroupName -ResourceType $resourceType -ResourceName $resourceName -Action list -ApiVersion 2015-08-01 -Force
        return $publishingCredentials
    }

    Publish()
    {
        $zipFile = "$($this.FunctionDirectory)\deploy.zip"
        try
        {
            $this.CreateZipPackage($zipFile)

            $publishingCredentials = $this.GetPublishingCredentials()
            $authToken = ("Basic {0}" -f [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes(("{0}:{1}" -f $publishingCredentials.Properties.PublishingUserName, $publishingCredentials.Properties.PublishingPassword))))
            $url = "https://{0}.scm.azurewebsites.net/api/zip/site/wwwroot/" -f $this.Name

            Write-Host "Publishing azure function: $($this.Name)"
            Invoke-RestMethod -Uri $url `
                              -Headers @{"Authorization"=$authToken;"If-Match"="*"} `
                              -Method PUT `
                              -InFile $zipFile `
                              -ContentType "multipart/form-data"
        }
        finally
        {
            remove-item -Path $zipFile
        }
    }

    Restart()
    {
        Write-Host "Restarting azure function: $($this.Name)"
        Restart-AzureRmWebApp -ResourceGroupName $this.ResourceGroupName -Name $this.Name
    }

    WriteLocalAppSettings([string] $DhaServiceBusListenConnectionString, [string] $DhaStorageConnectionString, [string] $IotHubServiceConnectionString)
    {
        $settings = @{
            "IsEncrypted" = $false; 
            "Values" =     $Values = @{
                "AzureWebJobsStorage" = "";
                "AzureWebJobsDashboard" =  "";
                "dha-servicebus" = "$DhaServiceBusListenConnectionString";
                "dha-storage" = "$DhaStorageConnectionString";
                "iothub-service" = "$IotHubServiceConnectionString"
            }
        }
        ConvertTo-Json $settings | out-file -FilePath "$($this.FunctionDirectory)\appsettings.json" -Encoding utf8 
    }
}

function Main
{
    Param([string] $ResourceGroupName, [string] $IotHubName, [string]$ServiceBusName, [string] $StorageName, [string] $AzureFunctionName)

    # Create DHA service bus namespace
    $serviceBus = [ServiceBus]::new($ServiceBusName, $ResourceGroupName)
    $serviceBus.CreateIfNotExist()

    # Create DHA service bus queue
    $queue = [ServiceBusQueue]::new("dha", $ServiceBusName, $ResourceGroupName)
    $queue.CreateIfNotExist()

    # Creating routes and routing rules for IoTHub
    $iotHub = [IoTHub]::new($IotHubName, $ResourceGroupName)
    $iotHub.AddServiceBusQueueEndpoint("dha", $queue.GetSendOnlyKey().PrimaryConnectionString, $ResourceGroupName, (Get-AzureRmContext).Subscription.Id)
    $iotHub.AddRoutingRule("dha", 'STARTS_WITH(MessageType, "DHA-")', $True, @("dha"), "DeviceMessages")

    # Create storage
    $storage = [Storage]::new($StorageName, $ResourceGroupName)
    $storage.CreateIfNotExist()

    # Gather the necessary connection strings for the DHA Azure function
    $DhaServiceBusListenConnectionString = $serviceBus.GetListenOnlyKey().PrimaryConnectionString
    $IotHubServiceConnectionString = $IotHub.GetServiceConnectionString().PrimaryConnectionString
    $DhaStorageConnectionString = $storage.GetConnectionString()

    # Work around for issue - https://github.com/Azure/azure-powershell/issues/3699
    $IotHubServiceConnectionString = $IotHubServiceConnectionString -replace "ShareAccessKeyName", "SharedAccessKeyName"

    # Create AzureFunction for DHA
    $dhaFunction = [AzureFunction]::new($AzureFunctionName, $ResourceGroupName)
    $dhaFunction.WriteLocalAppSettings($DhaServiceBusListenConnectionString, $DhaStorageConnectionString, $IotHubServiceConnectionString)
    $dhaFunction.CreateIfNotExist($DhaServiceBusListenConnectionString, $DhaStorageConnectionString, $IotHubServiceConnectionString)
    $dhaFunction.Publish()
    $dhaFunction.Restart()
}

#Starts here
Main -IotHubName $IotHubName `
     -ResourceGroupName $ResourceGroupName `
     -ServiceBusName $ServiceBusName `
     -StorageName $StorageName `
     -AzureFunctionName $AzureFunctionName 
