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

## Get Detailed Information About An Installed Certificate
To get more details about any of the installed certificates, the request can be initiated by calling `microsoft.management.getCertificateDetails` method.

### Input Payload 
<pre>
{
    "path" : "csp path"
    "hash" : "hashValue"
}
</pre>

### Output Payload
<pre>
{
    "Tag" : 52
    "Status" : value
    "IssuedTo" : "issued to value"
    "IssuedBy" : "issued by value"
    "ValidTo" : dateTime
    "ValidFrom" : dateTime
}
</pre>

##Examples:

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

### Retrieve Detailed Installed Certificate Information

If the operator wants to retrieve detailed information of certificate that has just been installed (in the example above), given its hash in the 'reported' properties section, the system should call `microsoft.management.getCertificateDetails` with the following parameters:

<pre>
{
    "path" : "./Device/Vendor/MSFT/RootCATrustedCertificates/Root"
    "hash" : "MyCertificateHash"
}
</pre>

The return payload should be:
<pre>
{
    "Tag" : 52
    "Status" : value
    "IssuedTo" : "issued to value"
    "IssuedBy" : "issued by value"
    "ValidTo" : "dateTime"
    "ValidFrom" : "dateTime"
}
</pre>
