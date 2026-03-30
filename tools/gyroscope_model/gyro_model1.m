function F = gyro_model1(p,t)
allan = p(1)./(t.^2)+p(2)./t+p(3)+p(4).*t+p(5).*t.^2;
F = allan;
