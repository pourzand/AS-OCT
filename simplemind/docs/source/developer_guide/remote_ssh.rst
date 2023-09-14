

************************************************
Bash_profile settings for remote SSH
************************************************
See Reference:

* https://code.visualstudio.com/docs/remote/ssh

To set keygen:

* https://code.visualstudio.com/docs/remote/troubleshooting#_configuring-key-based-authentication

Save this under ~/.ssh/config or write this in VS-code ssh setting

.. code-block:: console

  # Read more about SSH config files: https://linux.die.net/man/5/ssh_config
  # Lists
  # CVIB everybody, kanda weird sometimes
  # supernova.cvib.ucla.edu
  # 8 Tesla V100-SXM2-32GB
  Host supernova
    HostName REDLRADADM23589.ad.medctr.ucla.edu
    User youngwonchoi

  Host lambda1
    HostName REDLRADADM14958.ad.medctr.ucla.edu
    User youngwonchoi

  Host lambda2
    HostName REDLRADADM14959.ad.medctr.ucla.edu
    User youngwonchoi

  Host aldonova
    HostName REDLRADADM23710.ad.medctr.ucla.edu
    User youngwonchoi

  Host casanova
    HostName REDWRADMMC23199.ad.medctr.ucla.edu
    User youngwonchoi

  Host chevynova
    HostName REDLRADADM23712.ad.medctr.ucla.edu
    User youngwonchoi

