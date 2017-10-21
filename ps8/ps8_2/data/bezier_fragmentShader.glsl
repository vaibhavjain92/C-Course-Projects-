varying float ycoord;

void main()
{
	
	vec4 color;
	
	if(ycoord <= 0.0)
	{
		color = vec4(0.0, 0.0, 1.0, 1.0 );
	}	
	else
	{
		if(ycoord <= 0.25)
		{
			color = vec4(0.0, (ycoord)/0.25, 1.0, 1.0 );	
		}
		else
		{
			if(ycoord <= 0.50)
			{
				color = vec4(0.0, 1.0, 1.0 - (ycoord - 0.25)/0.25, 1.0 );
			}
			else
			{
				if(ycoord <= 0.75)
				{
					color = vec4((ycoord-0.5)/0.25, 1.0, 0.0, 1.0 );
				}
				else
				{
					if( ycoord <= 1.0)
					{
						color = vec4(1.0, 1.0-(ycoord-0.75)/0.25, 0.0, 1.0 );
					}
					else
					{
						color = vec4(1.0, 0.0, 0.0, 1.0 );
					}
				}
			}
		}
	}
	
    gl_FragColor=color;
}
