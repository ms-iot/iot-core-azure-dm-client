# Certificate Management

The **Certificate Management** functionality allows the operator to perform the following tasks:

- Install a certficate (from blob storage).
- Uninstall a certificate.
- List installed certificates.
- Get detailed information about an installed certificate.

To **install** new certificates, the operator will have to:

- Upload the certificate files to an Azure Storage account as blobs and save them in a container.
- Configure Azure storage is the device twin (see [externalStorage](external-storage.md)).
- Configure the certificate file names (and their containers) in the device twin.

Those tasks can be performed on a pre-defined set certificate stores. Target certificate stores are specified through their CSP paths. Below is a list of those CSPs path along with the corresponding json property name for each.

|    CSP Path |    Json Property                       |
|:-----------:|----------------------------------------|
| ./Device/Vendor/MSFT/RootCATrustedCertificates/Root             | rootCATrustedCertificates_Root             |
| ./Device/Vendor/MSFT/RootCATrustedCertificates/CA               | rootCATrustedCertificates_CA               |
| ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher | rootCATrustedCertificates_TrustedPublisher |
| ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople    | rootCATrustedCertificates_TrustedPeople    |
| ./Vendor/MSFT/CertificateStore/CA/System                        | certificateStore_CA_System                 |
| ./Vendor/MSFT/CertificateStore/Root/System                      | certificateStore_Root_System               |
| ./Vendor/MSFT/CertificateStore/My/User                          | certificateStore_My_User                   |
| ./Vendor/MSFT/CertificateStore/My/System                        | certificateStore_My_System                 |


- ./Device/Vendor/MSFT/RootCATrustedCertificates/Root **<-->** rootCATrustedCertificates_Root
- ./Device/Vendor/MSFT/RootCATrustedCertificates/CA **<-->** rootCATrustedCertificates_CA
- ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher **<-->** rootCATrustedCertificates_TrustedPublisher
- ./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople **<-->** rootCATrustedCertificates_TrustedPeople
- ./Vendor/MSFT/CertificateStore/CA/System **<-->** certificateStore_CA_System
- ./Vendor/MSFT/CertificateStore/Root/System **<-->** certificateStore_Root_System
- ./Vendor/MSFT/CertificateStore/My/User **<-->** certificateStore_My_User
- ./Vendor/MSFT/CertificateStore/My/System **<-->** certificateStore_My_System

## Install/Uninstall A Certificate
The operator can specify a list of certificates, and for each, what desired state is required.

<pre>
"desired": {
    "windows": {
        "certificates": {
            "rootCATrustedCertificates_Root": { <i>list of hashes and required states</i> },
            "rootCATrustedCertificates_CA": { <i>list of hashes and required states</i> },
            "rootCATrustedCertificates_TrustedPublisher": { <i>list of hashes and required states</i> },
            "rootCATrustedCertificates_TrustedPeople": { <i>list of hashes and required states</i> },
            "certificateStore_CA_System": { <i>list of hashes and required states</i> },
            "certificateStore_Root_System": { <i>list of hashes and required states</i> },
            "certificateStore_My_User": { <i>list of hashes and required states</i> },
            "certificateStore_My_System": { <i>list of hashes and required states</i> }
        }
    }
}
</pre>

#### List of Hashes and Required States:

<pre>
{
    "hash00": {
        "state": "installed"
        "fileName": "container00\\blobName.cer",
    }
    "hash01": {
        "state": "uninstalled"
        "fileName": "",
    }
}
</pre>

Notes:

- For the **install** scenario,
  - ```state``` is required and must be set to ```"installed"```.
  - ```fileName``` is required and must be in the form: <i>containerName\\blobName.cer</i>.
- For the **uninstall** scenario,
  - ```state``` is required and must be set to ```"uninstalled"```.
  - ```fileName``` is ignored.


## List Installed Certificates
The DM client reports the hashes of installed certificates under the same set of properties described above.

<pre>
"reported": {
    "windows": {
        "certificates": {
            "rootCATrustedCertificates_Root": { <i> reported hashes list </i> },
            "rootCATrustedCertificates_CA": { <i> reported hashes list </i> },
            "rootCATrustedCertificates_TrustedPublisher": { <i> reported hashes list </i> },
            "rootCATrustedCertificates_TrustedPeople": { <i> reported hashes list </i> },
            "certificateStore_CA_System": { <i> reported hashes list </i> },
            "certificateStore_Root_System": { <i> reported hashes list </i> },
            "certificateStore_My_User": { <i> reported hashes list </i> },
            "certificateStore_My_System": { <i> reported hashes list </i> }
        }
    }
}
</pre>

#### List of Reported Hashes:

<pre>
{
    "hash00" : "",
    "hash01" : ""
}
</pre>


## Retrieve Certificate Details
To get more details about any of the installed certificates, the request can be initiated by calling the asynchronous `microsoft.windows.getCertificateDetails` method.
The method will schedule a job on the device to capture the certificate details in a json file and upload it to the specified blob in Azure Storage.
The method returns immediately and indicates that it has accepted or rejected the job.

### Input Payload 

<pre>
{
    "path" : "csp path",
    "hash" : "hashValue",
    "connectionString" : "connectionStrng",
    "containerName" : "containerName",
    "output" : "blobFileName"
}
</pre>

## Output Payload
The device responds immediately with the following JSON payload:

<pre>
{
    "response" : value (See below)
    "reason" : value (See below)
}
</pre>

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
    "windows": {
        "certificates": {
            "rootCATrustedCertificates_Root": "fileName01.cer/fileName02.cer/MyCertificate.cer",
            "rootCATrustedCertificates_CA": { <i>list of hashes and required states</i> }
            "rootCATrustedCertificates_TrustedPublisher": { <i>list of hashes and required states</i> }
            "rootCATrustedCertificates_TrustedPeople": { <i>list of hashes and required states</i> }
            "certificateStore_CA_System": { <i>list of hashes and required states</i> }
            "certificateStore_Root_System": { <i>list of hashes and required states</i> }
            "certificateStore_My_User": { <i>list of hashes and required states</i> }
            "certificateStore_My_System": "fileName01.cer/fileName02.cer"
        }
    }
}
</pre>

----

[Home Page](../README.md) | [Library Reference](library-reference.md)