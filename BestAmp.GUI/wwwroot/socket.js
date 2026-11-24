window.tcpClient = {
    sock: null,
    dotNetObject: null,

    /** Connects to the TCP server */
    connect: function (url, dotNetObject) {
        this.dotNetObject = dotNetObject;
        this.sock = new WebSocket(url);

        // https://developer.mozilla.org/docs/Web/API/WebSocket/binaryType
        this.sock.binaryType = "arraybuffer";

        this.sock.onopen = () => {
            console.log("Connected to local bridge");
            this.dotNetObject.invokeMethodAsync("OnBridgeConnected");
        };

        this.sock.onmessage = event => {
            const data = new Uint8Array(event.data);

            // needs to match method name in Home.razor
            this.dotNetObject.invokeMethodAsync("OnTcpMessage", data);
        };

        this.sock.onclose = () => {
            console.log("Disconnected cleanly");
        };
    },

    /** Sends data to TCP server */
    send: function (data) {
        if (this.sock?.readyState === WebSocket.OPEN) {
            this.sock.send(new Uint8Array(data));
        }
    },

    /** Closes the client */
    close: function () {
        this.sock?.close();
        this.sock = null;
    }
};