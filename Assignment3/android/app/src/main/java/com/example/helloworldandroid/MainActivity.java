package com.example.helloworldandroid;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MainActivity – GLPI Framework Intro demo.
 *
 * Renders the fan (base/pole/hub/4 blades) using OpenGL ES 3.0 and
 * forwards touch gestures (tap = toggle, drag = speed boost) to the
 * native Renderer via JNI. Every native call from onTouchEvent is
 * marshalled onto the GL thread with glSurfaceView.queueEvent(...) so
 * it is safe to touch GL/Model state from there.
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "GLPIFrameworkIntro";

    static {
        System.loadLibrary("glpiframeworkintro");
    }

    private GLSurfaceView glSurfaceView;
    private boolean rendererInitialized = false;

    private native boolean nativeInit(AssetManager assetManager);
    private native void    nativeResize(int width, int height);
    private native void    nativeRender();
    private native void    nativeTouchDown(float x, float y);
    private native void    nativeTouchMove(float x, float y);
    private native void    nativeTouchUp();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d(TAG, "onSurfaceCreated");
                boolean ok = nativeInit(getAssets());
                if (ok) {
                    rendererInitialized = true;
                    Log.d(TAG, "Renderer initialized");
                } else {
                    Log.e(TAG, "Renderer initialization failed");
                }
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Log.d(TAG, "onSurfaceChanged: " + width + "x" + height);
                if (rendererInitialized) nativeResize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) nativeRender();
            }
        });

        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {
        if (!rendererInitialized) return super.onTouchEvent(event);

        final float x = event.getX();
        final float y = event.getY();

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                glSurfaceView.queueEvent(new Runnable() {
                    @Override public void run() { nativeTouchDown(x, y); }
                });
                return true;
            case MotionEvent.ACTION_MOVE:
                glSurfaceView.queueEvent(new Runnable() {
                    @Override public void run() { nativeTouchMove(x, y); }
                });
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                glSurfaceView.queueEvent(new Runnable() {
                    @Override public void run() { nativeTouchUp(); }
                });
                return true;
            default:
                return super.onTouchEvent(event);
        }
    }
    @Override
    protected void onPause()  { super.onPause();  glSurfaceView.onPause();  }

    @Override
    protected void onResume() { super.onResume(); glSurfaceView.onResume(); }
}
