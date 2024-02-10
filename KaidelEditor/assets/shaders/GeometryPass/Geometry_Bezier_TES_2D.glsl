#version 460 core

layout(isolines) in;

uniform mat4 u_MVP;
uniform int u_NumControlPoints;

int binomial(int n, int k) {
    int res = 1;
    if(k>n-k)
        k = n-k;
    for (int i = 0; i < k; ++i) {
        res *= (n - i);
        res /= (i + 1);
    }
    return res;
}

layout(location = 0)in vec3 v_VSPositions[];


void main(){
    int numControlPoints = u_NumControlPoints;

	float numLines = gl_TessLevelOuter[0];
	float numSegmentsPerLine = gl_TessLevelOuter[1];

    float segmentCoord = gl_TessCoord.x;
	float lineCoord = gl_TessCoord.y;

	int lineIndex = int(lineCoord * numLines);
	int segmentIndexInLine = int(segmentCoord * numSegmentsPerLine);

	int segmentIndex = int(segmentIndexInLine + lineIndex * numSegmentsPerLine);



	float t = float(segmentIndex)/(numLines*numSegmentsPerLine);
    vec3 result = vec3(0.0);
    float oneMinusT = 1.0 - t;

     for (int i = 0; i < numControlPoints; ++i) {
        float b;
        if (i == 0 && t == 0.0) {
            b = 1.0; // Handle the case when i = 0 and t = 0
        } else if (i == numControlPoints - 1 && t == 1.0) {
            b = 1.0; // Handle the case when i = numControlPoints - 1 and t = 1
        } else {
            b = float(binomial(numControlPoints - 1, i)) * pow(t, float(i)) * pow(oneMinusT, float(numControlPoints - 1 - i));
        }
        result += v_VSPositions[i] * b;
    }

    gl_Position = u_MVP * vec4(result,1.0);
}
