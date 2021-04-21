// JavaScript Document
Vector3D = function Vector3D(x, y, z){
	this.x = x || 0;
	this.y = y || 0;
	this.z = z || 0;
}
Vector3D.distance = function(vector1, vector2){
	var xdiff = vector1.x - vector2.x;
	var ydiff = vector1.y - vector2.y;
	var zdiff = vector1.z - vector2.z;
	return Math.sqrt((xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff));
}
Vector3D.prototype.length = function(){
	return Math.sqrt((this.x * this.x) + (this.y * this.y) + (this.z * this.z));
}
Vector3D.prototype.lengthSquared = function(){
	return (this.x * this.x) + (this.y * this.y) + (this.z * this.z);
}
Vector3D.prototype.normalize = function(){
	var len = this.length();
	this.x /= len;
	this.y /= len;
	this.z /= len;
	return len;
}
Vector3D.prototype.incrementBy = function(vector3d){
	this.x += vector3d.x;
	this.y += vector3d.y;
	this.z += vector3d.z;
}
Vector3D.prototype.decrementBy = function(vector3d){
	this.x -= vector3d.x;
	this.y -= vector3d.y;
	this.z -= vector3d.z;
}
Vector3D.prototype.scaleBy = function(scalar){
	this.x *= scalar;
	this.y *= scalar;
	this.z *= scalar;
}
Vector3D.prototype.negate = function(){
	this.x *= -1;
	this.y *= -1;
	this.z *= -1;
}
Vector3D.prototype.equals = function(vector3d){
	return this.x == vector3d.x && this.y == vector3d.y && this.z == vector3d.z;
}
Vector3D.prototype.clone = function(){
	return new Vector3D(this.x, this.y, this.z);
}
Vector3D.prototype.add = function(vector3d){
	return new Vector3D(this.x+vector3d.x, this.y+vector3d.y, this.z+vector3d.z);
}
Vector3D.prototype.subtract = function(vector3d){
	return new Vector3D(this.x-vector3d.x, this.y-vector3d.y, this.z-vector3d.z);
}
Vector3D.prototype.fastSubtract = function(vector3d, toCache){
	toCache.x = this.x-vector3d.x;
	toCache.y = this.y-vector3d.y;
	toCache.z = this.z-vector3d.z;
}