package uk.co.arcadebadgers.androidexample;

public class MainActivity extends android.app.NativeActivity {
	static {
		System.loadLibrary("stlport_shared");
		System.loadLibrary("glesgae");
		System.loadLibrary("native-activity");
	}
}
