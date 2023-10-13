Import("env")

# see https://github.com/platformio/platformio-core/issues/3742#issuecomment-1003454439
def wait_for_monitor_port(source, target, env):
    # "pio test" has no delay between upload & monitoring. Unfortuneatly, the teensy
    # is rebooting at that point and the port isn't available. This rasies an exception. 
    port = env.GetProjectOption("monitor_port")
    if port is None:
        from platformio.builder.tools.pioupload import AutodetectUploadPort
        AutodetectUploadPort(env)
        port = env.subst("$UPLOAD_PORT")
    if port:
        # We have a port specified, wait for it to
        # activate
        print(f"Waiting for port {port}...")
        import serial
        while True:
            try:
                serial.Serial(port)
                print("...done!")
                return
            except:
                pass

    # No port specified, try a generic delay
    print("Delay while uploading...")
    import time
    time.sleep(2)
    print("...done!")

env.AddPostAction("upload", wait_for_monitor_port)