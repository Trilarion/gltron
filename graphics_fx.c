#include "gltron.h"

void drawGlow(Player *pCam, Player *pTarget, gDisplay *d, float dim) {
  float mat[4*4];
  float alpha, dist;

  glPushMatrix();
  glTranslatef(pTarget->data->posx,
	       pTarget->data->posy,
	       0);

  glShadeModel(GL_SMOOTH);

  glDepthMask(GL_FALSE);
  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  glGetFloatv(GL_MODELVIEW_MATRIX, mat);
  mat[0] = mat[5] = mat[10] = 1.0;
  mat[1] = mat[2] = 0.0;
  mat[4] = mat[6] = 0.0;
  mat[8] = mat[9] = 0.0;
  glLoadMatrixf(mat);

	{
		vec3 v;
		v.v[0] = pTarget->data->posx - pCam->camera->cam[0];
		v.v[1] = pTarget->data->posy - pCam->camera->cam[1];
		v.v[2] = 0 - pCam->camera->cam[2];
		dist = vec3Length(&v);
	}
	{
	  const float fMin = 30;
		const float fMax = 100;
		if(dist < fMin) alpha = 0.0f;
		else if(dist > fMax) alpha = 1.0f;
		else alpha = (dist - fMin) / (fMax - fMin);
		// printf("dist: %.2f, alpha: %.2f\n", dist, alpha);
	}
  glBegin(GL_TRIANGLE_FAN);
  glColor4f(pTarget->pColorDiffuse[0], 
						pTarget->pColorDiffuse[1], 
						pTarget->pColorDiffuse[2],
						alpha);

  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);
  glVertex3f(dim*cos(1.0*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(1.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(2.0*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(2.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(3.0*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(3.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(4.0*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(4.0*3.1415/5.0), 0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glEnd();
  polycount += 5;


  glBegin(GL_TRIANGLES);
  glColor4f(pTarget->pColorDiffuse[0], 
						pTarget->pColorDiffuse[1], 
						pTarget->pColorDiffuse[2],
						alpha);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glVertex3f(dim*cos(-0.2*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(-0.2*3.1415/5.0), 0);

  glColor4f(pTarget->pColorDiffuse[0], 
						pTarget->pColorDiffuse[1], 
						pTarget->pColorDiffuse[2],
						alpha);
  glVertex3f(0,TRAIL_HEIGHT/2, 0);
  glColor4f(0,0,0,0.0);
  glVertex3f(dim*cos(5.2*3.1415/5.0),
	     TRAIL_HEIGHT/2+dim*sin(5.2*3.1415/5.0), 0);
  glVertex3f(0,-TRAIL_HEIGHT/4,0);
  glEnd();
  polycount += 3;

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  glShadeModel( game->screen->shademodel );
  glPopMatrix();  
}

void drawImpact(Player *p) {

  glDisable(GL_LIGHTING);
  glPushMatrix();

  glRotatef(90, 90, 0, 1);

  /*
   * Drop impact slightly below the floor and move it a tiny bit
   * back from the impact point so we don't draw behind the
   * trail/wall that's hit.
   */
  glTranslatef(0.0, -0.5, -0.5);

  glColor3f(0.68, 0, 0);
  
  drawExplosion(&(p->data->impact_radius));
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

