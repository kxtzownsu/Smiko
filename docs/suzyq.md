# SuzyQable
At one point in time Google used to produce cables capable of interfacing with the Google Security Chip over USB-C. Some people still sell boards capable of recreating this behavior on Ebay, and the cable itself provides a full USB console to the GSC as well as USB transfer capabilities. Smiko is capable of interfacing with these methods to perform certain actions. 

(If you don't already have a SuzyQ, I _strongly_ recommend getting one for debugging, brick protection, and exploitation)


To utilize Smiko over SuzyQ, simply add the `--suzyq` or `-s` parameter to all operations. For example, to read the sysinfo over SuzyQ, run as follows:
```bash
sudo smiko --sysinfo --suzyq
```