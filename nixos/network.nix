{ config, pkgs, ... }:

{
  networking.hostName = "naxce";
  networking.networkmanager.enable = true;

  networking.wireless.enable = true;
  boot.extraModprobeConfig = ''
    options cfg80211 ieee80211_regdom=PL
  '';
  networking.networkmanager.wifi.scanRandMacAddress = false;

  networking.networkmanager.unmanaged = [ "interface-name:wlp16s0" ];
}
