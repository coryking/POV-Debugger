#!/usr/bin/env python3

Import("env", "projenv")
import subprocess

def before_build(source, target, env):
    protoc_command = "/opt/homebrew/bin/protoc"
    nanopb_plugin = "/opt/homebrew/bin/protoc-gen-nanopb"
    proto_files_dir = "./proto"
    output_dir = "./lib"
    subprocess.run([
        protoc_command,
        f"--plugin=protoc-gen-nanopb={nanopb_plugin}",
        f"--nanopb_out={output_dir}",
        f"{proto_files_dir}/*.proto"
    ], check=True)

env.AddPreAction("buildprog", before_build)
