{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-24.05";

  outputs = {
    self,
    nixpkgs,
  }: let
    pkgs = import nixpkgs {
      system = "x86_64-linux";
    };
  in {
    devShells.${pkgs.system}.default = pkgs.mkShell {
      packages = with pkgs; [
        gnumake
        pkg-config
        clang-tools
        clang
        bear
      ];
    };
  };
}
