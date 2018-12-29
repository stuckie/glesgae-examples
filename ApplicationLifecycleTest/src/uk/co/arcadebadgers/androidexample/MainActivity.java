package uk.co.arcadebadgers.androidexample;

public class MainActivity extends android.app.NativeActivity {
	static {
		System.loadLibrary("glesgae");
		System.loadLibrary("native-activity");
	}
}
