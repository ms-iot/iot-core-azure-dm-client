# Certificate Management

The **Certificate Management** functionality allows the operator to perform the following tasks:
- Install a certficate (from blob storage).
- Uninstall a certificate.
- List installed certificates.
- Get detailed information about an installed certificate.

Those tasks can be performed on a pre-defined set of CSP paths. Below is a list of those CSPs path along with the corresponding json property name for each.

- ./Device/Vendor/MSFT/RootCATrustedCertificates/Root **<-->** rootCATrustedCertificates_Root
- ./Device/Vendor/MSFT/RootCATrustedCertificates/CA **<-->** rootCATrustedCertificates_CA
- ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher **<-->** rootCATrustedCertificates_TrustedPublisher
- ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople **<-->** rootCATrustedCertificates_TrustedPeople
- ./Vendor/MSFT/CertificateStore/CA/System **<-->** certificateStore_CA_System
- ./Vendor/MSFT/CertificateStore/Root/System **<-->** certificateStore_Root_System
- ./Vendor/MSFT/CertificateStore/My/User **<-->** certificateStore_My_User
- ./Vendor/MSFT/CertificateStore/My/System **<-->** certificateStore_My_System

## Install/Uninstall A Certificate
The operator can specify the list of desired certificates to be installed under a certain CSP path. When the device receives the desired state, it compares it to the device current state and:
- If a certificate exists in the desired list but is not installed on the device, it is installed.
- If a certificate is installed on the device, but is not present in the desired list, it is uninstalled.

<pre>
    "desired": {
      "microsoft": {
        "management": {
          "certificates": {
            "rootCATrustedCertificates_Root": "fileName01.cer/fileName02.cer",
            "rootCATrustedCertificates_CA": "fileName01.cer/fileName02.cer",
            "rootCATrustedCertificates_TrustedPublisher": "fileName01.cer/fileName02.cer",
            "rootCATrustedCertificates_TrustedPeople": "fileName01.cer/fileName02.cer",
            "certificateStore_CA_System": "fileName01.cer/fileName02.cer",
            "certificateStore_Root_System": "fileName01.cer/fileName02.cer",
            "certificateStore_My_User": "fileName01.cer/fileName02.cer",
            "certificateStore_My_System": "fileName01.cer/fileName02.cer"
          },
        }
      }
    }
</pre>

## List Installed Certificates
The DM client reports the hashes of installed certificates under the pre-defined set of CSP paths. Each set of hashes will appear under the json property name corresponding to the CSPs path as described in the mapping above.

<pre>
    "reported": {
      "microsoft": {
        "management": {
          "certificates": {
            "Tag": 50,
            "Status": 0,
            "Configuration": {
            "rootCATrustedCertificates_Root": "hash00/hash01",
            "rootCATrustedCertificates_CA": "hash00/hash01",
            "rootCATrustedCertificates_TrustedPublisher": "hash00/hash01",
            "rootCATrustedCertificates_TrustedPeople": "hash00/hash01",
            "certificateStore_CA_System": "hash00/hash01",
            "certificateStore_Root_System": "hash00/hash01",
            "certificateStore_My_User": "hash00/hash01",
            "certificateStore_My_System": "hash00/hash01",
            }
          }
        }
      }
    }
</pre>

## Retrieve Certificate Details
To get more details about any of the installed certificates, the request can be initiated by calling the asynchronous `microsoft.management.getCertificateDetails` method.
The method will schedule a job on the device to capture the certificate details in a json file and upload it to the specified blob in Azure Storage.
The method returns immediately and indicates that it has accepted or rejected the job.

### Input Payload 
```
{
    "path" : "csp path",
    "hash" : "hashValue",
    "connectionString" : "connectionStrng",
    "containerName" : "containerName",
    "output" : "blobFileName"
}
````

## Output Payload
The device responds immediately with the following JSON payload:

```
{
    "response" : value (See below)
    "reason" : value (See below)
}
```

Possible `"response"` values are: 
- `"accepted"` - The reboot request was accepted. The device will retrieve the certificate details and upload it to the Azure Storage specified in the input parameters.
- `"rejected"` - The device rejected the request.

`"reason"` is used to communicate why an App Install request was rejected if possible.

### Uploaded File Format
<pre>
{
    "Tag" : 52,
    "Status" : value,
    "Base64Encoding" : "Base64Encoding value",
	"TemplateName" : "TemplateName",
    "IssuedTo" : "issued to value",
    "IssuedBy" : "issued by value",
    "ValidTo" : "date time value",
    "ValidFrom" : "date time value"
}
</pre>

## Examples:

### Install Certificates

If the operator wants to install a new certificate (MyCertificate.cer) to ./Device/Vendor/MSFT/RootCATrustedCertificates/Root, the following steps should be followed:
- Upload the certificate file to the default Azure blob storage. Let's assume its hash is MyCertificateHash.
- Set the desired properties to:
<pre>
    "desired": {
      "microsoft": {
        "management": {
          "certificates": {
            "rootCATrustedCertificates_Root": "fileName01.cer/fileName02.cer/MyCertificate.cer",
            "rootCATrustedCertificates_CA": "fileName01.cer/fileName02.cer",
            "rootCATrustedCertificates_TrustedPublisher": "fileName01.cer/fileName02.cer",
            "rootCATrustedCertificates_TrustedPeople": "fileName01.cer/fileName02.cer",
            "certificateStore_CA_System": "fileName01.cer/fileName02.cer",
            "certificateStore_Root_System": "fileName01.cer/fileName02.cer",
            "certificateStore_My_User": "fileName01.cer/fileName02.cer",
            "certificateStore_My_System": "fileName01.cer/fileName02.cer"
          },
        }
      }
    }
</pre>
